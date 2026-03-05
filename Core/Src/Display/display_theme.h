/*
 * display_theme.h - H735G-DK adaptation (480x272 RK043FN48H)
 */
#ifndef SRC_DISPLAY_DISPLAY_THEME_H_
#define SRC_DISPLAY_DISPLAY_THEME_H_

#include "stm32_lcd.h"

#define LCD_WIDTH       480
#define LCD_HEIGHT      272
#define PAGE_START_X    8
#define PAGE_START_Y    46
#define PAGE_START_LINE 3
#define PAGE_END_X      472
#define PAGE_END_Y      265
#define PAGE_END_LINE   18
#define PAGE_WIDTH      (PAGE_END_X - PAGE_START_X)
#define BACKGROUND      UTIL_LCD_COLOR_DARKCYAN
#define FOREGROUND      UTIL_LCD_COLOR_WHITE

#endif /* SRC_DISPLAY_DISPLAY_THEME_H_ */