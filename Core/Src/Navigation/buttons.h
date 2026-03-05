/*
 * buttons.h – Button initialisation for STM32H735G-DK (no joystick, one USER button)
 *
 * Navigation is performed via UART remote_input (numpad keys).
 * The USER button (PC13, EXTI15_10) toggles navigation mode.
 */

#ifndef SRC_NAVIGATION_BUTTONS_H_
#define SRC_NAVIGATION_BUTTONS_H_

#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "state_machine.h"

void initButtons(void)
{
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
}

void BSP_PB_Callback(Button_TypeDef Button)
{
    toggle_nav_mode();
}

#endif /* SRC_NAVIGATION_BUTTONS_H_ */
