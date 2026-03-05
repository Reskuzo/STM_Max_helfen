/*
 * peripherals.h – Peripheral initialisation for STM32H735G-DK
 *   H735G-DK has 2 LEDs (LED1=green, LED2=red) and no hardware joystick.
 */

#ifndef SRC_CONFIG_PERIPHERALS_H_
#define SRC_CONFIG_PERIPHERALS_H_

#include <remote_input.h>
#include "../Navigation/buttons.h"
#include "../Display/display_main.h"
#include "../Audio/audio_main.h"

void init_peripherals(void)
{
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);

    BSP_LED_Off(LED_RED);

    /* USER button (no joystick on H735G-DK – navigation via UART) */
    initButtons();

    /* PC button simulation via USART1 (ST-Link VCP) */
    initUartInput();

    /* Display */
    display_default_page();

    /* Audio */
    Audio_SETUP();
}

#endif /* SRC_CONFIG_PERIPHERALS_H_ */
