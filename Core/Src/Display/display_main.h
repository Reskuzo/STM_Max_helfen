/*
 * display_main.h - H735G-DK adaptation
 */
#ifndef SRC_DISPLAY_DISPLAY_MAIN_H_
#define SRC_DISPLAY_DISPLAY_MAIN_H_

#include "stdlib.h"
#include <stdint.h>

extern uint8_t is_init;

void display_default_page(void);
void display_navigation(char *current, char *left, char *right);
void display_toolbar(char *left, char *right, char *up, char *down, char *center);
void display_latency(uint32_t latency_ms);
void println(uint8_t line, char *string);

#endif /* SRC_DISPLAY_DISPLAY_MAIN_H_ */