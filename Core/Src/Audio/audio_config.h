/*
 * audio_config.h - H735G-DK: SAI4 PDM Instance 1 (same as H747-DISCO)
 */
#ifndef SRC_AUDIO_AUDIO_CONFIG_H_
#define SRC_AUDIO_AUDIO_CONFIG_H_

#include <stdint.h>
#include <float.h>
#include "stm32h735g_discovery_audio.h"
#include "noise_filter.h"

/* External reference to BSP Audio context */
extern AUDIO_IN_Ctx_t Audio_In_Ctx[];

/* Buffer sizes - identical to H747 project */
#define AUDIO_IN_PDM_BUFFER_SIZE  1024  /* PDM samples (16-bit words) per DMA half */
#define AUDIO_IN_PCM_BUFFER_SIZE  512   /* mono PCM samples per processing frame   */
#define RECORD_BUFFER_SIZE        4096
#define AUDIO_INSTANCE            1     /* Instance 1 = SAI4 PDM (same as H747)    */

/* PDM recording buffer in D3 SRAM (BDMA can only access D3 domain) */
extern uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE];

/* Audio config globals */
extern uint32_t AudioFreq[9];
extern uint32_t *AudioFreq_ptr;
extern BSP_AUDIO_Init_t AudioInInit;
extern BSP_AUDIO_Init_t AudioOutInit;
extern uint32_t VolumeLevel;

/* Processed mono audio buffers */
extern int16_t   PcmAudioBuffer[];
extern float     FrequencyBuffer[];

extern volatile uint32_t buffer_index;
extern int audio_ready;
extern volatile uint32_t audio_capture_tick;

/* =============================================================================== Settings */
#define DO_PDM_PCM_TRANSFORM 0
#define APPLY_PCM_SIG_FILTER 1
#define DO_FOURIER_TRANSFORM 2
#define APPLY_FOURIER_FILTER 3
#define DO_INVERSE_TRANSFORM 4

extern uint8_t audio_transforms;

uint8_t processing_step_enabled(uint8_t step);

/* PDM -> PCM conversion (called from DMA callbacks) */
void process_PDM_to_PCM(uint32_t startEntryOffset);

/* Callbacks */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance);

/* Processing */
void process_audio(void);
void apply_FFT(void);
void apply_inverse_FFT(void);

#endif /* SRC_AUDIO_AUDIO_CONFIG_H_ */