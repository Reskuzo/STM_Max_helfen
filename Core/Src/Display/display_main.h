/*
 * display_main.h
 *
 *  Created on: Feb 18, 2026
 *      Author: yannick
 */

#ifndef SRC_DISPLAY_DISPLAY_MAIN_H_
#define SRC_DISPLAY_DISPLAY_MAIN_H_

#include "stdlib.h"
#include <stdint.h>

static uint8_t is_init = 0;

/** @brief Set the default theming with background colour border and default headline
 */
void display_default_page();

void display_navigation(char* current, char* left, char* right);

void display_toolbar(char* left, char* right, char* up, char* down, char* center);

void display_latency(uint32_t latency_ms);

void println(uint8_t line, char *string);

#endif /* SRC_DISPLAY_DISPLAY_MAIN_H_ */
