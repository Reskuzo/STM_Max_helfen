/*
 * audio_main.h
 */

#ifndef SRC_AUDIO_AUDIO_MAIN_H_
#define SRC_AUDIO_AUDIO_MAIN_H_

#include "audio_config.h"

extern void (*onAudioReady)(void);
extern uint8_t is_recording;

void Audio_SETUP(void);
void start_recording(void);
void stop_recording(void);
void apply_volume(void);

#endif /* SRC_AUDIO_AUDIO_MAIN_H_ */
