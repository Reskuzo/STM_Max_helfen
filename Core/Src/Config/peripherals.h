/*
 * peripherals.h - H735G-DK adaptation
 * H735G-DK has only 2 LEDs: LED1 (GREEN), LED2 (RED).
 * No hardware joystick — BUTTON_USER + UART remote input only.
 */
#ifndef SRC_CONFIG_PERIPHERALS_H_
#define SRC_CONFIG_PERIPHERALS_H_

#include "../Navigation/remote_input.h"
#include "../Navigation/buttons.h"
#include "../Display/display_main.h"
#include "../Audio/audio_main.h"

void init_peripherals(void)
{
    BSP_LED_Init(LED1);  /* GREEN */
    BSP_LED_Init(LED2);  /* RED   */

    BSP_LED_Off(LED1);
    BSP_LED_Off(LED2);

    /* BUTTON_USER (EXTI) */
    initButtons();

    /* PC remote control via USART3 / ST-Link VCP */
    initUartInput();

    /* LCD */
    display_default_page();

    /* DFSDM audio (Instance 2) */
    Audio_SETUP();
}

#endif /* SRC_CONFIG_PERIPHERALS_H_ */