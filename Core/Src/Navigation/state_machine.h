/*
 * state_machine.h
 */
#ifndef SRC_NAVIGATION_STATE_MACHINE_H_
#define SRC_NAVIGATION_STATE_MACHINE_H_

#include <stdint.h>

/* Joystick value aliases (H735 has no HW joystick, defined here for compatibility) */
#define JOY_SEL   0x01U
#define JOY_DOWN  0x02U
#define JOY_LEFT  0x04U
#define JOY_RIGHT 0x08U
#define JOY_UP    0x10U

#include "../Games/snake/snake.h"

typedef struct {
    char name[50];
    void (*onInit)(void);
    void (*onTick)(void);
    void (*onJoyLeft)(void);
    void (*onJoyRight)(void);
    void (*onJoyUp)(void);
    void (*onJoyDown)(void);
    void (*onJoyCenter)(void);
    char textJoyLeft[50];
    char textJoyRight[50];
    char textJoyUp[50];
    char textJoyDown[50];
    char textJoyCenter[50];
} state_t;

typedef struct {
    char name[50];
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
void toggle_nav_mode(void);
void onProgramStart(void);
void onTick(void);

#endif /* SRC_NAVIGATION_STATE_MACHINE_H_ */