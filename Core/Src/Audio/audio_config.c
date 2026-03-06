/*
 * audio_config.c - H735G-DK: SAI4 PDM Instance 1 (same flow as H747-DISCO)
 */
#define ARM_MATH_CM7
#include "audio_config.h"
#include "stm32h735g_discovery.h"
#include "main.h"
#include "arm_math.h"
#include <math.h>

/* PDM buffer in D3 SRAM - BDMA (used by SAI4) can only access D3 domain */
ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".RAM_D3")));

/* Audio configuration globals */
uint32_t AudioFreq[9] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
uint32_t *AudioFreq_ptr;
BSP_AUDIO_Init_t AudioInInit;
BSP_AUDIO_Init_t AudioOutInit;
uint32_t VolumeLevel = 100;

ALIGN_32BYTES (int16_t PcmAudioBuffer[AUDIO_IN_PCM_BUFFER_SIZE]);
volatile uint32_t buffer_index = 0;
int audio_ready = 0;
volatile uint32_t audio_capture_tick = 0;

/* FFT magnitude output */
float FrequencyBuffer[AUDIO_IN_PCM_BUFFER_SIZE / 2];

/* Audio transform pipeline bitmask */
uint8_t audio_transforms = 0              +
        (1 << DO_PDM_PCM_TRANSFORM) +
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

/* PDM_RAW_DIAGNOSTIC mode (set to 1 to bypass the PDM library):
 *   Displays bit-density of each raw DMA halfword, scaled to ±32000.
 *   All-zeros buffer  → flat line at BOTTOM  (−32000): DATA pin held low,
 *                        SAI4 not receiving bits (check PDM clock / mic power).
 *   All-0xFFFF buffer → flat line at TOP     (+32000): DATA pin pulled high,
 *                        PDM clock NOT reaching the mic (check PE2 / SAI4 MCLK).
 *   Noisy signal near center                          : PDM clock is working,
 *                        mic is outputting data — issue is in PDM→PCM filter.
 * Set back to 0 once the root cause is found. */
#define PDM_RAW_DIAGNOSTIC 1

/* Convert one PDM half-buffer to PCM and accumulate into PcmAudioBuffer ring */
void process_PDM_to_PCM(uint32_t startEntryOffset)
{
    if (!processing_step_enabled(DO_PDM_PCM_TRANSFORM)) return;

    uint16_t *startEntry = &recordPDMBuf[startEntryOffset];
    int16_t temp_pcm[32]; /* 16 stereo pairs interleaved */

#if PDM_RAW_DIAGNOSTIC
    /* BIT-DENSITY MODE: count '1' bits per halfword, centre at 8.
     * Scale by 4000 so the result fills ~±32000 of the int16 range.
     *   all-0x0000 → popcount=0  → (0-8)*4000 = -32000  (bottom of display)
     *   all-0xFFFF → popcount=16 → (16-8)*4000= +32000  (top of display)
     *   PDM silence → popcount≈8 → ≈0              (noisy centre)         */
    for (uint32_t i = 0; i < 16; i++) {
        int16_t density = (int16_t)(__builtin_popcount(startEntry[i * 2])     - 8) * 4000;
        temp_pcm[i * 2]     = density;
        temp_pcm[i * 2 + 1] = (int16_t)(__builtin_popcount(startEntry[i * 2 + 1]) - 8) * 4000;
    }
#else
    BSP_AUDIO_IN_PDMToPCM(AUDIO_INSTANCE, startEntry, (uint16_t *)temp_pcm);
#endif

    /* Average L+R into mono ring buffer */
    for (uint32_t i = 0; i < 16; i++) {
        PcmAudioBuffer[buffer_index] = (temp_pcm[i * 2] + temp_pcm[i * 2 + 1]) / 2;
        buffer_index++;
        if (buffer_index >= AUDIO_IN_PCM_BUFFER_SIZE) {
            buffer_index = 0;
            audio_capture_tick = HAL_GetTick();
            audio_ready = 1;
        }
    }
}

void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
    /* BSP SAI RX callbacks always pass Instance=0 regardless of SAI instance used.
     * Since there is only one audio input, process unconditionally. */
    (void)Instance;
    process_PDM_to_PCM(0);
}

void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
    /* See HalfTransfer note above — BSP hardcodes Instance=0 for SAI RX path. */
    (void)Instance;
    BSP_LED_Toggle(LED1);
    process_PDM_to_PCM(AUDIO_IN_PDM_BUFFER_SIZE / 2);
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