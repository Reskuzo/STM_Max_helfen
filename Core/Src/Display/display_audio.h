/*
 * display_audio.h - H735G-DK adaptation
 */
#ifndef SRC_DISPLAY_DISPLAY_AUDIO_H_
#define SRC_DISPLAY_DISPLAY_AUDIO_H_

#include <stdint.h>

#define WAVE_CENTER_Y   136
#define WAVE_AMPLITUDE   50

void Draw_AudioWaveform(int16_t *pData, uint32_t Size);

#endif /* SRC_DISPLAY_DISPLAY_AUDIO_H_ */