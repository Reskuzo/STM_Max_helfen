/*
 * audio_main.c – Audio setup and control for STM32H735G-DK
 */

#include "audio_main.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_audio.h"
#include "main.h"

uint8_t is_recording = 0;

void Audio_SETUP(void)
{
    int32_t result;

    /* CRC clock required by PDM conversion library */
    __HAL_RCC_CRC_CLK_ENABLE();

    /* Configure digital PDM microphone input at 16 kHz, 2 channels */
    AudioFreq_ptr          = AudioFreq + 2;   /* 16000 Hz */
    AudioInInit.Device     = AUDIO_IN_DEVICE_DIGITAL_MIC;
    AudioInInit.ChannelsNbr = 2;
    AudioInInit.SampleRate  = *AudioFreq_ptr;
    AudioInInit.BitsPerSample = AUDIO_RESOLUTION_16B;
    AudioInInit.Volume      = VolumeLevel;

    result = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &AudioInInit);
    if (result != BSP_ERROR_NONE) {
        BSP_LED_On(LED_RED);
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
        Error_Handler();
}

void stop_recording(void)
{
    if (is_recording == 0) return;
    BSP_AUDIO_IN_Pause(AUDIO_INSTANCE);
    audio_ready   = 0;
    is_recording  = 0;
}

void apply_volume(void)
{
    /* H735G uses BSP_AUDIO_IN_SetVolume (0–100 scale) */
    BSP_AUDIO_IN_SetVolume(AUDIO_INSTANCE, VolumeLevel);
}
