/*
 * buttons.h - H735G-DK adaptation (no joystick, BUTTON_USER only)
 */
#ifndef SRC_NAVIGATION_BUTTONS_H_
#define SRC_NAVIGATION_BUTTONS_H_

#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "state_machine.h"

/* External variables from main.c */
extern int16_t noise_gate_threshold;
extern volatile int snake_launch_requested;

void initButtons(void)
{
    /* H735G-DK has no joystick – only BUTTON_USER (PC13, EXTI15_10) */
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
}

void BSP_PB_Callback(Button_TypeDef Button)
{
    (void)Button;
    toggle_nav_mode();
}

#endif /* SRC_NAVIGATION_BUTTONS_H_ */