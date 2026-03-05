/**
  ******************************************************************************
  * @file    main.c
  * @brief   Main program for STM32H735G-DK sound analysis (single-core Cortex-M7)
  ******************************************************************************
  */

#include "main.h"
#include <stdio.h>
#include "Clock/clock_config.h"
#include "Config/peripherals.h"
#include "Navigation/state_machine.h"
#include "Display/display_audio.h"

int main(void)
{
  MPU_Config();
  CPU_CACHE_Enable();
  HAL_Init();
  SystemClock_Config();

  init_peripherals();
  onProgramStart();

  while (1)
  {
    onTick();
  }
}

void Error_Handler(void)
{
  BSP_LED_Off(LED_GREEN);
  BSP_LED_On(LED_RED);
  while(1);
}
