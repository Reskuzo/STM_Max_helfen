/*
 * audio_main.c - H735G-DK: DFSDM Instance 2 (digital microphones)
 * No PDM->PCM conversion needed; DFSDM delivers 16-bit stereo PCM directly.
 */
#include "audio_main.h"
#include "stm32h735g_discovery.h"
#include "main.h"

uint8_t is_recording = 0;

void Audio_SETUP(void)
{
    int32_t result;

    /* Configure audio input for 2 digital mics via DFSDM Instance 2, 16 kHz */
    AudioFreq_ptr = AudioFreq + 2;  /* index 2 = 16000 Hz */

    AudioInInit.Device        = AUDIO_IN_DEVICE_DIGITAL_MIC;
    AudioInInit.ChannelsNbr   = 2;
    AudioInInit.SampleRate    = *AudioFreq_ptr;
    AudioInInit.BitsPerSample = AUDIO_RESOLUTION_16B;
    AudioInInit.Volume        = VolumeLevel;

    result = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &AudioInInit);
    if (result != BSP_ERROR_NONE)
    {
        BSP_LED_On(LED2);  /* LED2 = RED on H735G-DK */
        Error_Handler();
    }
}

void start_recording(void)
{
    if (is_recording++ > 0) return;
    /* dfsdmStereoBuffer is circular double-buffered; size in bytes */
    uint32_t status = BSP_AUDIO_IN_Record(AUDIO_INSTANCE,
                                          (uint8_t *)dfsdmStereoBuffer,
                                          sizeof(dfsdmStereoBuffer));
    if (status != BSP_ERROR_NONE)
    {
        Error_Handler();
    }
}

void stop_recording(void)
{
    if (is_recording == 0) return;
    BSP_AUDIO_IN_Stop(AUDIO_INSTANCE);
    audio_ready  = 0;
    is_recording = 0;
}

void apply_volume(void)
{
    BSP_AUDIO_IN_SetVolume(AUDIO_INSTANCE, VolumeLevel);
}