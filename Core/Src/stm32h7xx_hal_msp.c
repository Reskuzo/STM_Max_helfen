/**
  * @file    stm32h7xx_hal_msp.c
  * @brief   HAL MSP initialization for H735G-DK BSP-based project.
  *          Minimal implementation - BSP drivers handle their own MSP callbacks.
  */
#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}