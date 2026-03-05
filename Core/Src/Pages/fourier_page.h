/*
 * fourier_page.h
 *
 *  Created on: Feb 20, 2026
 *      Author: yannick
 */

#ifndef SRC_PAGES_FOURIER_PAGE_H_
#define SRC_PAGES_FOURIER_PAGE_H_

#include <stdint.h>

extern uint16_t freq_cursor;
extern uint8_t  fourier_hold;

void fourier_init(void);
void fourier_onTick(void);
void fourier_cursor_left(void);
void fourier_cursor_right(void);
void fourier_toggle_hold(void);
void fourier_toggle_scale(void);

#endif /* SRC_PAGES_FOURIER_PAGE_H_ */
