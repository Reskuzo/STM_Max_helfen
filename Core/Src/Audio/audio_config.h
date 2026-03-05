/*
 * audio_config.h – Audio buffer and processing config for STM32H735G-DK
 */

#ifndef SRC_AUDIO_AUDIO_CONFIG_H_
#define SRC_AUDIO_AUDIO_CONFIG_H_

#include <stdint.h>
#include <float.h>
#include "stm32h735g_discovery_audio.h"
#include "noise_filter.h"

/* External reference to BSP Audio context */
extern AUDIO_IN_Ctx_t Audio_In_Ctx[];

/* Private defines */
#define AUDIO_IN_PDM_BUFFER_SIZE  1024
#define AUDIO_IN_PCM_BUFFER_SIZE  512
#define RECORD_BUFFER_SIZE        4096
#define AUDIO_INSTANCE            1

/* Audio buffers in D3 SRAM for BDMA access */
extern uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE];

/* Audio state */
extern uint32_t AudioFreq[9];
extern uint32_t *AudioFreq_ptr;
extern BSP_AUDIO_Init_t AudioInInit;
extern uint32_t VolumeLevel;

/* Audio processing buffers */
extern int16_t PcmAudioBuffer[];
extern float   FrequencyBuffer[];

extern volatile uint32_t buffer_index;
extern int audio_ready;
extern volatile uint32_t audio_capture_tick;

/* Processing pipeline step flags */
#define DO_PDM_PCM_TRANSFORM 0
#define APPLY_PCM_SIG_FILTER 1
#define DO_FOURIER_TRANSFORM 2
#define APPLY_FOURIER_FILTER 3
#define DO_INVERSE_TRANSFORM 4

static uint8_t audio_transforms = 0
    + (true << DO_PDM_PCM_TRANSFORM)
    + (true << DO_FOURIER_TRANSFORM)
    + (true << DO_INVERSE_TRANSFORM)
    + (true << APPLY_FOURIER_FILTER)
    + (true << APPLY_PCM_SIG_FILTER);

uint8_t processing_step_enabled(uint8_t step);

/* DMA callbacks (implemented in audio_config.c) */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance);

/* Processing functions */
void process_audio(void);
void process_PDM_to_PCM(uint32_t startEntryOffset);
void apply_FFT(void);
void apply_inverse_FFT(void);

#endif /* SRC_AUDIO_AUDIO_CONFIG_H_ */
