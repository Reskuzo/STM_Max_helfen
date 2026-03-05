/*
 * display_theme.h – Display layout constants for STM32H735G-DK (480×272 LCD)
 */

#ifndef SRC_DISPLAY_DISPLAY_THEME_H_
#define SRC_DISPLAY_DISPLAY_THEME_H_

#include "stm32_lcd.h"

#define LCD_WIDTH       480
#define LCD_HEIGHT      272
#define PAGE_START_X    8
#define PAGE_START_Y    32
#define PAGE_START_LINE 2
#define PAGE_END_X      471
#define PAGE_END_Y      252
#define PAGE_END_LINE   14
#define PAGE_WIDTH      (PAGE_END_X - PAGE_START_X)
#define BACKGROUND      UTIL_LCD_COLOR_DARKCYAN
#define FOREGROUND      UTIL_LCD_COLOR_WHITE

#endif /* SRC_DISPLAY_DISPLAY_THEME_H_ */
