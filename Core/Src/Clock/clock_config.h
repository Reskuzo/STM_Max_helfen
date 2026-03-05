/*
 * clock_config.h – System clock declarations for STM32H735G-DK
 *   SYSCLK = 520 MHz (HSE 25 MHz, PLL1 M=5 N=104 P=1)
 *   HCLK   = 260 MHz, APBx = 130 MHz
 */

#ifndef SRC_CLOCK_CLOCK_CONFIG_H_
#define SRC_CLOCK_CLOCK_CONFIG_H_

#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery_audio.h"

void SystemClock_Config(void);
void MPU_Config(void);
void CPU_CACHE_Enable(void);

/**
  * @brief  Override BSP SAI4 Block A init to configure PDM mode.
  */
HAL_StatusTypeDef MX_SAI4_Block_A_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config_t *MXConfig);

#endif /* SRC_CLOCK_CLOCK_CONFIG_H_ */
