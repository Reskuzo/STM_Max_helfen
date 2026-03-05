/*
 * remote_input.h - H735G-DK adaptation
 * UART remote control via USART3 (VCP connector: PD8=TX, PD9=RX)
 * Key mapping (sent from PC, numpad):
 *   '8' -> JOY_UP    (0x10)
 *   '2' -> JOY_DOWN  (0x02)
 *   '4' -> JOY_LEFT  (0x04)
 *   '6' -> JOY_RIGHT (0x08)
 *   '5' -> JOY_SEL   (0x01)
 *   CR  -> toggle navigation mode
 */
#ifndef SRC_NAVIGATION_REMOTE_INPUT_H_
#define SRC_NAVIGATION_REMOTE_INPUT_H_

#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "state_machine.h"

#define ENABLE_REMOTE_INPUT 1

UART_HandleTypeDef huart_input;
static uint8_t uart_rx_byte;

void initUartInput(void)
{
    if (!ENABLE_REMOTE_INPUT) return;

    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    /* PD8 = USART3_TX, PD9 = USART3_RX, AF7 */
    gpio.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &gpio);

    huart_input.Instance          = USART3;
    huart_input.Init.BaudRate     = 115200;
    huart_input.Init.WordLength   = UART_WORDLENGTH_8B;
    huart_input.Init.StopBits     = UART_STOPBITS_1;
    huart_input.Init.Parity       = UART_PARITY_NONE;
    huart_input.Init.Mode         = UART_MODE_TX_RX;
    huart_input.Init.HwFlowCtl   = UART_HWCONTROL_NONE;
    huart_input.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart_input);

    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    /* Arm the first receive */
    HAL_UART_Receive_IT(&huart_input, &uart_rx_byte, 1);
}

/* Called by HAL after each received byte */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (!ENABLE_REMOTE_INPUT) return;
    if (huart->Instance == USART3)
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