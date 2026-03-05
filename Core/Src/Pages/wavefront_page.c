/*
 * wavefront_page.c
 *
 *  Created on: Feb 20, 2026
 *      Author: yannick
 */
#include "wavefront_page.h"
#include "../Display/display_audio.h"
#include "../Display/display_main.h"
#include "../Audio/audio_main.h"
#include "../Audio/noise_filter.h"
#include "main.h"
#include <stdio.h>

uint32_t increase = 1;
uint8_t thres_volume_toggle = 0;




void wavefront_init(){

	// set the pages theme to the default theme
	display_default_page();

	// set the processing level and start recording
	audio_transforms =
			(1  << DO_PDM_PCM_TRANSFORM) +
			(processing_step_enabled(APPLY_FOURIER_FILTER)  << DO_FOURIER_TRANSFORM) + // only do the Fourier pipeline if fourier filtering is enabled
			(1  << DO_INVERSE_TRANSFORM) ; // will only be performed if fft is active

	start_recording();

}


void wavefront_onTick(){

	if (!audio_ready) return;
	uint32_t t0 = audio_capture_tick;
	process_audio();
	audio_ready = 0;

	Draw_AudioWaveform(PcmAudioBuffer, AUDIO_IN_PCM_BUFFER_SIZE);
	display_latency(HAL_GetTick() - t0);

	// print the threshold below the data
	 char data_str[100];
	 sprintf(data_str, "Threshold: %d, Volume: %lu, +- %lu", intensity_threshold , VolumeLevel, increase);
	 println(24, data_str);

}

void raise_noise_threshold(){

	if (thres_volume_toggle) intensity_threshold += increase;
	else { VolumeLevel += increase; apply_volume(); }
}
void lower_noise_threshold(){
	if (thres_volume_toggle) intensity_threshold -= increase;
	else { VolumeLevel -= increase; apply_volume(); }
}

void toggle_thres_vol(){
	thres_volume_toggle = !thres_volume_toggle;
}

void raise_inc(){
	increase *= 2;
}
void lower_inc(){
	increase /= 2;
	if (increase < 1) increase = 1;
}

