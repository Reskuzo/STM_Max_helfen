/*
 * state_machine.h
 *
 *  Created on: Feb 15, 2026
 *      Author: yannick
 */

#ifndef SRC_NAVIGATION_STATE_MACHINE_H_
#define SRC_NAVIGATION_STATE_MACHINE_H_

#include <stdint.h>
#include "../Games/snake/snake.h"

/* Joystick direction codes – used by both hardware (H747I) and UART remote input */
#define JOY_SEL   0x00000001U
#define JOY_DOWN  0x00000002U
#define JOY_LEFT  0x00000004U
#define JOY_RIGHT 0x00000008U
#define JOY_UP    0x00000010U

typedef struct {
	uint8_t name[50];
	void (*onInit)(void);
	void (*onTick)(void);
	void (*onJoyLeft)(void);
	void (*onJoyRight)(void);
	void (*onJoyUp)(void);
	void (*onJoyDown)(void);
	void (*onJoyCenter)(void);
	uint8_t textJoyLeft[50];
	uint8_t textJoyRight[50];
	uint8_t textJoyUp[50];
	uint8_t textJoyDown[50];
	uint8_t textJoyCenter[50];
} state_t;

typedef struct {
	uint8_t name[50];
	void (*start_func)(void);
	void (*joy_input_forewarding)(uint32_t joyPin);
} game_t;

extern uint8_t navigation_mode;
extern uint8_t game_state;
extern uint8_t state;

void blank(void);

extern state_t pages[];
extern uint8_t num_pages;
extern game_t games[];


void joy_forwarding(uint32_t joypin);
void toggle_nav_mode();

void onTick();


#endif /* SRC_NAVIGATION_STATE_MACHINE_H_ */
