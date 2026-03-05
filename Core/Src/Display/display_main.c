/*
 * display_main.c – LCD initialisation and UI helpers for STM32H735G-DK (480×272)
 */

#include "display_main.h"
#include "display_theme.h"
#include "stm32h735g_discovery_lcd.h"
#include "../../../Utilities/lcd/stm32_lcd.h"
#include <string.h>


void display_default_page(void)
{
    if (!is_init) {
        BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
        UTIL_LCD_SetFuncDriver(&LCD_Driver);
        is_init = 1;
    }

    uint32_t x_size, y_size;
    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    UTIL_LCD_SetFont(&Font16);
    UTIL_LCD_SetBackColor(BACKGROUND);
    UTIL_LCD_Clear(BACKGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);

    /* Two-line header */
    UTIL_LCD_DisplayStringAt(0, 2,  (uint8_t *)"Uni Bayreuth - Mikrocontroller Praktikum II", CENTER_MODE);
    UTIL_LCD_DisplayStringAt(0, 18, (uint8_t *)"Sound Analysis - Yannick Pahlke", CENTER_MODE);

    /* Content border */
    UTIL_LCD_DrawRect(PAGE_START_X, LINE(PAGE_START_LINE),
                      x_size - PAGE_START_X * 2,
                      y_size - LINE(PAGE_START_LINE) - LINE(PAGE_END_LINE - PAGE_START_LINE + 1),
                      FOREGROUND);
}

void println(uint8_t line, char *string)
{
    UTIL_LCD_FillRect(PAGE_START_X, LINE(line), PAGE_WIDTH, 15, UTIL_LCD_COLOR_DARKCYAN);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    UTIL_LCD_DisplayStringAt(PAGE_START_X, LINE(line), (uint8_t *)string, LEFT_MODE);
}

void display_navigation(char *current, char *left, char *right)
{
    char nav_str[120];
    sprintf(nav_str, "< %-8.8s   %-10.10s   %-8.8s >", left, current, right);
    println(PAGE_END_LINE, nav_str);
}

void display_toolbar(char *left, char *right, char *up, char *down, char *center)
{
    char tool_str[120];
    sprintf(tool_str, "< %-6.6s  v %-6.6s  o Nav  + %-6.6s  ^ %-6.6s  %-6.6s >",
            left, down, center, up, right);
    println(PAGE_END_LINE, tool_str);
}

void display_latency(uint32_t latency_ms)
{
    char buf[16];
    sprintf(buf, "%lums", latency_ms);
    UTIL_LCD_FillRect(PAGE_END_X - 60, LINE(PAGE_END_LINE), 60, 15, BACKGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);
    UTIL_LCD_DisplayStringAt(PAGE_END_X - 60, LINE(PAGE_END_LINE), (uint8_t *)buf, LEFT_MODE);
}
