/*
 * audio_config.c - H735G-DK adaptation (DFSDM Instance 2, no PDM->PCM conversion)
 */
#define ARM_MATH_CM7
#include "audio_config.h"
#include "stm32h735g_discovery.h"
#include "main.h"
#include "arm_math.h"
#include <math.h>

/* DFSDM stereo circular DMA buffer (double-buffered): L0,R0,L1,R1,...
 * Half: samples 0..DFSDM_STEREO_BUF_SAMPLES-1 filled first
 * Full: samples DFSDM_STEREO_BUF_SAMPLES..end filled next */
int16_t dfsdmStereoBuffer[DFSDM_STEREO_BUF_SAMPLES * 2];

/* Audio configuration globals */
uint32_t AudioFreq[9] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
uint32_t *AudioFreq_ptr;
BSP_AUDIO_Init_t AudioInInit;
BSP_AUDIO_Init_t AudioOutInit;
uint32_t VolumeLevel = 80;

ALIGN_32BYTES (int16_t PcmAudioBuffer[AUDIO_IN_PCM_BUFFER_SIZE]);
volatile uint32_t buffer_index = 0;
int audio_ready = 0;
volatile uint32_t audio_capture_tick = 0;

/* FFT magnitude output */
float FrequencyBuffer[AUDIO_IN_PCM_BUFFER_SIZE / 2];

/* Audio transform pipeline bitmask */
uint8_t audio_transforms = 0              +
        (1 << DO_PDM_PCM_TRANSFORM) +   /* kept for API compatibility */
        (1 << DO_FOURIER_TRANSFORM) +
        (1 << DO_INVERSE_TRANSFORM) +
        (1 << APPLY_FOURIER_FILTER) +
        (1 << APPLY_PCM_SIG_FILTER) ;

/* Internal complex FFT buffer */
static float32_t fft_complex_buf[512];
static arm_rfft_fast_instance_f32 fft_instance;
static uint8_t fft_init_done = 0;

static void ensure_fft_init(void)
{
    if (!fft_init_done) {
        arm_rfft_fast_init_f32(&fft_instance, 512);
        fft_init_done = 1;
    }
}

/* Convert stereo int16 half-buffer to mono PCM */
static void dfsdm_to_mono(const int16_t *stereo, uint32_t stereo_pairs, int16_t *mono_out)
{
    for (uint32_t i = 0; i < stereo_pairs; i++) {
        mono_out[i] = (int16_t)(((int32_t)stereo[2*i] + (int32_t)stereo[2*i+1]) >> 1);
    }
}

/* DFSDM half-transfer: first half of stereo buffer ready */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    if (Instance == AUDIO_INSTANCE) {
        dfsdm_to_mono(&dfsdmStereoBuffer[0], AUDIO_IN_PCM_BUFFER_SIZE / 2,
                      &PcmAudioBuffer[0]);
    }
}

/* DFSDM full-transfer: second half of stereo buffer ready */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    if (Instance == AUDIO_INSTANCE) {
        BSP_LED_Toggle(LED1);
        dfsdm_to_mono(&dfsdmStereoBuffer[AUDIO_IN_PCM_BUFFER_SIZE],
                      AUDIO_IN_PCM_BUFFER_SIZE / 2,
                      &PcmAudioBuffer[AUDIO_IN_PCM_BUFFER_SIZE / 2]);
        audio_capture_tick = HAL_GetTick();
        audio_ready = 1;
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    (void)Instance;
    Error_Handler();
}

void apply_FFT(void)
{
    ensure_fft_init();
    static float32_t input[AUDIO_IN_PCM_BUFFER_SIZE];
    arm_q15_to_float((q15_t*)PcmAudioBuffer, input, 512);
    arm_rfft_fast_f32(&fft_instance, input, fft_complex_buf, 0);
    for (uint32_t i = 0; i < 256; i++) {
        float32_t re = fft_complex_buf[i * 2];
        float32_t im = fft_complex_buf[i * 2 + 1];
        float32_t mag;
        arm_sqrt_f32(re * re + im * im, &mag);
        FrequencyBuffer[i] = mag;
    }
}

void apply_inverse_FFT(void)
{
    ensure_fft_init();
    static float32_t output[512];
    arm_rfft_fast_f32(&fft_instance, fft_complex_buf, output, 1);
    arm_float_to_q15(output, (q15_t*)PcmAudioBuffer, 512);
}

uint8_t processing_step_enabled(uint8_t step)
{
    return (audio_transforms & (1u << step)) > 0;
}

void process_audio(void)
{
    if (!audio_ready) return;
    if (processing_step_enabled(APPLY_PCM_SIG_FILTER)) apply_filters(PcmAudioBuffer, AUDIO_IN_PCM_BUFFER_SIZE);
    if (processing_step_enabled(DO_FOURIER_TRANSFORM))  apply_FFT(); else return;
    if (processing_step_enabled(APPLY_FOURIER_FILTER))  apply_fft_filters(FrequencyBuffer, AUDIO_IN_PCM_BUFFER_SIZE/2, 16000); else return;
    if (processing_step_enabled(DO_INVERSE_TRANSFORM))  apply_inverse_FFT();
}