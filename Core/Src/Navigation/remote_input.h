/*
 * remote_input.h – UART-based remote control for STM32H735G-DK
 *
 * Allows navigation from a PC serial terminal (numpad keys) – identical
 * to the H747I-DISCO version but without the H747I discovery include.
 *
 * Key mapping (numpad, sent via UART):
 *   '8' -> JOY_UP    (0x10)
 *   '2' -> JOY_DOWN  (0x02)
 *   '4' -> JOY_LEFT  (0x04)
 *   '6' -> JOY_RIGHT (0x08)
 *   '5' -> JOY_SEL   (0x01)
 *   CR  -> push button (toggle navigation mode)
 */

#ifndef SRC_NAVIGATION_REMOTE_INPUT_H_
#define SRC_NAVIGATION_REMOTE_INPUT_H_

#include "stm32h7xx_hal.h"
#include "state_machine.h"

/* JOY pin values are defined in state_machine.h */

#define ENABLE_REMOTE_INPUT 1

UART_HandleTypeDef huart_input;
static uint8_t uart_rx_byte;

void initUartInput(void)
{
    if (!ENABLE_REMOTE_INPUT) return;
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart_input.Instance          = USART1;
    huart_input.Init.BaudRate     = 115200;
    huart_input.Init.WordLength   = UART_WORDLENGTH_8B;
    huart_input.Init.StopBits     = UART_STOPBITS_1;
    huart_input.Init.Parity       = UART_PARITY_NONE;
    huart_input.Init.Mode         = UART_MODE_TX_RX;
    huart_input.Init.HwFlowCtl   = UART_HWCONTROL_NONE;
    huart_input.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart_input);

    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    HAL_UART_Receive_IT(&huart_input, &uart_rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (!ENABLE_REMOTE_INPUT) return;
    if (huart->Instance == USART1)
    {
        switch (uart_rx_byte)
        {
            case '8':  joy_forwarding(JOY_UP);    break;
            case '2':  joy_forwarding(JOY_DOWN);  break;
            case '4':  joy_forwarding(JOY_LEFT);  break;
            case '6':  joy_forwarding(JOY_RIGHT); break;
            case '5':  joy_forwarding(JOY_SEL);   break;
            case '\r': toggle_nav_mode();          break;
            default:   break;
        }
        HAL_UART_Receive_IT(&huart_input, &uart_rx_byte, 1);
    }
}

#endif /* SRC_NAVIGATION_REMOTE_INPUT_H_ */
