/*
 * audio_main.c - H735G-DK: SAI4 PDM Instance 1 (same as H747-DISCO)
 */
#include "audio_main.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_audio.h"
#include "main.h"

uint8_t is_recording = 0;

void Audio_SETUP(void)
{
    int32_t result;

    /* Enable CRC – required by the PDM-to-PCM library */
    __HAL_RCC_CRC_CLK_ENABLE();

    /* Configure audio input: 1 digital microphone via SAI4 PDM, 16 kHz */
    AudioFreq_ptr = AudioFreq + 2;  /* index 2 = 16000 Hz */

    AudioInInit.Device        = AUDIO_IN_DEVICE_DIGITAL_MIC1;
    AudioInInit.ChannelsNbr   = 2;
    AudioInInit.SampleRate    = *AudioFreq_ptr;
    AudioInInit.BitsPerSample = AUDIO_RESOLUTION_16B;
    AudioInInit.Volume        = VolumeLevel;

    result = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &AudioInInit);
    if (result != BSP_ERROR_NONE)
    {
        BSP_LED_On(LED2);
        Error_Handler();
    }
}

void start_recording(void)
{
    if (is_recording++ > 0) return;
    uint32_t status = BSP_AUDIO_IN_RecordPDM(AUDIO_INSTANCE,
                                              (uint8_t *)&recordPDMBuf,
                                              AUDIO_IN_PDM_BUFFER_SIZE * sizeof(uint16_t));
    if (status != BSP_ERROR_NONE)
    {
        Error_Handler();
    }
}

void stop_recording(void)
{
    if (is_recording == 0) return;
    BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    audio_ready  = 0;
    is_recording = 0;
}

void apply_volume(void)
{
    BSP_AUDIO_IN_SetVolume(AUDIO_INSTANCE, VolumeLevel);
}