/**
  ******************************************************************************
  * @file    stm32h735g_discovery_conf.h
  * @brief   STM32H735G-DK board configuration file for sound analysis project.
  ******************************************************************************
  */

#ifndef STM32H735G_DK_CONF_H
#define STM32H735G_DK_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"

/* COM feature not used */
#define USE_COM_LOG                         0U
#define USE_BSP_COM_FEATURE                 0U

/* LCD controller */
#define USE_LCD_CTRL_RK043FN48H             1U

/* LCD frame buffer in OctoSPI RAM */
#define LCD_LAYER_0_ADDRESS                 0x70000000U
#define LCD_LAYER_1_ADDRESS                 0x70200000U

/* Audio codec */
#define USE_AUDIO_CODEC_WM8994              1U
/* Enable PDM library support (needed for BSP_AUDIO_IN_RecordPDM / PDMToPCM) */
#define USE_BSP_PDM_LIB_FEATURE             1U

/* Default Audio IN internal buffer size */
#define DEFAULT_AUDIO_IN_BUFFER_SIZE        2048U

/* Touch screen (required for LCD BSP to compile) */
#define USE_TS_GESTURE                      1U
#define USE_TS_MULTI_TOUCH                  1U
#define TS_TOUCH_NBR                        2U

/* IRQ priorities */
#define BSP_BUTTON_USER_IT_PRIORITY         15U
#define BSP_AUDIO_OUT_IT_PRIORITY           14U
#define BSP_AUDIO_IN_IT_PRIORITY            15U
#define BSP_SD_IT_PRIORITY                  14U
#define BSP_SD_RX_IT_PRIORITY               14U
#define BSP_SD_TX_IT_PRIORITY               15U
#define BSP_TS_IT_PRIORITY                  15U
#define BSP_OSPI_NOR_IT_PRIORITY            6U
#define BSP_OSPI_RAM_IT_PRIORITY            7U

#ifdef __cplusplus
}
#endif

#endif /* STM32H735G_DK_CONF_H */
