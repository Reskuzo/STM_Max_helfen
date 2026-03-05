/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt handlers for STM32H735G-DK (no joystick, one user button)
  ******************************************************************************
  */

#include "main.h"
#include "stm32h7xx_it.h"

extern UART_HandleTypeDef huart_input;

/* Cortex-M7 core exceptions */
void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void)  { while (1) {} }
void UsageFault_Handler(void){ while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

/**
  * @brief  EXTI15_10 IRQ handler – BUTTON_USER (PC13) on H735G-DK
  */
void EXTI15_10_IRQHandler(void)
{
  BSP_PB_IRQHandler(BUTTON_USER);
}

/**
  * @brief  USART1 IRQ handler – ST-Link VCP / remote control
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart_input);
}

/**
  * @brief  SAI DMA IRQ handler for audio OUT (instance 0)
  */
void AUDIO_OUT_SAIx_DMAx_IRQHandler(void)
{
  BSP_AUDIO_OUT_IRQHandler(0);
}

/**
  * @brief  BDMA Channel 1 IRQ handler – SAI4 PDM audio IN (instance 1)
  */
void BDMA_Channel1_IRQHandler(void)
{
  BSP_AUDIO_IN_IRQHandler(1, AUDIO_IN_DEVICE_DIGITAL_MIC);
}
