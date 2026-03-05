/*
 * audio_config.c – PDM-to-PCM conversion and FFT processing (STM32H735G-DK)
 */

#define ARM_MATH_CM7
#include "audio_config.h"
#include "stm32h735g_discovery.h"
#include "main.h"
#include "arm_math.h"
#include <math.h>

/* Audio PDM buffer in D3 SRAM – required for BDMA access */
ALIGN_32BYTES(uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));

uint32_t  AudioFreq[9] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
uint32_t *AudioFreq_ptr;
BSP_AUDIO_Init_t AudioInInit;
uint32_t  VolumeLevel = 100;

ALIGN_32BYTES(int16_t PcmAudioBuffer[AUDIO_IN_PCM_BUFFER_SIZE]);
volatile uint32_t buffer_index      = 0;
int               audio_ready       = 0;
volatile uint32_t audio_capture_tick = 0;

float FrequencyBuffer[AUDIO_IN_PCM_BUFFER_SIZE / 2];

static float32_t            fft_complex_buf[512];
static arm_rfft_fast_instance_f32 fft_instance;
static uint8_t              fft_init_done = 0;

static void ensure_fft_init(void)
{
    if (!fft_init_done) {
        arm_rfft_fast_init_f32(&fft_instance, 512);
        fft_init_done = 1;
    }
}

void process_PDM_to_PCM(uint32_t startEntryOffset)
{
    if (!processing_step_enabled(DO_PDM_PCM_TRANSFORM)) return;

    uint16_t *startEntry = &recordPDMBuf[startEntryOffset];
    int16_t   temp_pcm[32]; /* 16 samples * 2 channels interleaved */
    BSP_AUDIO_IN_PDMToPCM(AUDIO_INSTANCE, startEntry, (uint16_t *)temp_pcm);

    for (uint32_t i = 0; i < 16; i++) {
        PcmAudioBuffer[buffer_index] = (temp_pcm[i * 2] + temp_pcm[i * 2 + 1]) / 2;
        buffer_index++;
        if (buffer_index >= 512) {
            buffer_index        = 0;
            audio_capture_tick  = HAL_GetTick();
            audio_ready         = 1;
        }
    }
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    if (Instance == 1U)
        process_PDM_to_PCM(0);
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    if (Instance == 1U) {
        BSP_LED_Toggle(LED_GREEN);   /* H735G has no LED_BLUE */
        process_PDM_to_PCM(AUDIO_IN_PDM_BUFFER_SIZE / 2);
    }
}

void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
    Error_Handler();
}

void apply_FFT(void)
{
    ensure_fft_init();
    static float32_t input[AUDIO_IN_PCM_BUFFER_SIZE];
    arm_q15_to_float((q15_t *)PcmAudioBuffer, input, 512);
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
    arm_float_to_q15(output, (q15_t *)PcmAudioBuffer, 512);
}

uint8_t processing_step_enabled(uint8_t step)
{
    return (audio_transforms & (true << step)) > 0;
}

void process_audio(void)
{
    if (!audio_ready) return;
    if (processing_step_enabled(APPLY_PCM_SIG_FILTER)) apply_filters(PcmAudioBuffer, AUDIO_IN_PCM_BUFFER_SIZE);
    if (processing_step_enabled(DO_FOURIER_TRANSFORM))  apply_FFT(); else return;
    if (processing_step_enabled(APPLY_FOURIER_FILTER))  apply_fft_filters(FrequencyBuffer, AUDIO_IN_PCM_BUFFER_SIZE / 2, 16000); else return;
    if (processing_step_enabled(DO_INVERSE_TRANSFORM))  apply_inverse_FFT();
}
