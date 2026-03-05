/**
  ******************************************************************************
  * @file    main.h
  * @brief   Header for main.c (STM32H735G-DK port)
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#include "stdio.h"
#include "string.h"
#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_lcd.h"
#include "stm32h735g_discovery_conf.h"
#include "stm32h735g_discovery_audio.h"
#include "stm32_lcd.h"

/* LED aliases – H735G-DK has only two LEDs */
#define LED_GREEN  LED1
#define LED_RED    LED2

/* Error handler */
void Error_Handler(void);

#endif /* __MAIN_H */
