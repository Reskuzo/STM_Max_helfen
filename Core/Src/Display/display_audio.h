/*
 * display_audio.h – Waveform display for 480×272 LCD
 */

#ifndef SRC_DISPLAY_DISPLAY_AUDIO_H_
#define SRC_DISPLAY_DISPLAY_AUDIO_H_

#include <stdint.h>

#define WAVE_CENTER_Y   136    /* vertical centre for 272 px display */
#define WAVE_AMPLITUDE   60    /* max pixel deviation from centre     */

void Draw_AudioWaveform(int16_t *pData, uint32_t Size);

#endif /* SRC_DISPLAY_DISPLAY_AUDIO_H_ */
