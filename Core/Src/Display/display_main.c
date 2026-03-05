/*
 * display_main.c - H735G-DK adaptation (480x272 LCD)
 */
#include "display_main.h"

uint8_t is_init = 0;
#include "display_theme.h"
#include "stm32h735g_discovery_lcd.h"
#include "../../../Utilities/lcd/stm32_lcd.h"
#include <string.h>
#include <stdio.h>

void display_default_page(void)
{
    if (!is_init)
    {
        BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
        UTIL_LCD_SetFuncDriver(&LCD_Driver);
        is_init = 1;
    }
    UTIL_LCD_SetFont(&UTIL_LCD_DEFAULT_FONT);

    uint32_t x_size, y_size;
    BSP_LCD_GetXSize(0, &x_size);
    BSP_LCD_GetYSize(0, &y_size);

    UTIL_LCD_SetBackColor(BACKGROUND);
    UTIL_LCD_Clear(BACKGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);

    UTIL_LCD_DisplayStringAt(0, 5,  (uint8_t *)"Uni Bayreuth - MC Praktikum II", CENTER_MODE);
    UTIL_LCD_DisplayStringAt(0, 20, (uint8_t *)"Sound Analysis - H735G-DK port", CENTER_MODE);

    UTIL_LCD_SetFont(&Font16);
    UTIL_LCD_DrawRect(4, LINE(2), x_size - 8, y_size - LINE(3) - 4, FOREGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);
}

void println(uint8_t line, char *string)
{
    UTIL_LCD_FillRect(PAGE_START_X, LINE(line), PAGE_WIDTH, 15, BACKGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);
    UTIL_LCD_DisplayStringAt(PAGE_START_X, LINE(line), (uint8_t *)string, LEFT_MODE);
}

void display_navigation(char *current, char *left, char *right)
{
    char nav_str[200];
    snprintf(nav_str, sizeof(nav_str), "< %-8.8s  o Toools  %-8.8s  + Games  %-8.8s>",
             left, current, right);
    println(PAGE_END_LINE - 1, nav_str);
}

void display_toolbar(char *left, char *right, char *up, char *down, char *center)
{
    char tool_str[200];
    snprintf(tool_str, sizeof(tool_str), "< %-6.6s  v %-6.6s  o Nav  + %-6.6s  ^ %-6.6s  %-6.6s >",
             left, down, center, up, right);
    println(PAGE_END_LINE - 1, tool_str);
}

void display_latency(uint32_t latency_ms)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%lums", latency_ms);
    UTIL_LCD_FillRect(PAGE_END_X - 60, LINE(PAGE_END_LINE - 1), 60, 15, BACKGROUND);
    UTIL_LCD_SetTextColor(FOREGROUND);
    UTIL_LCD_DisplayStringAt(PAGE_END_X - 60, LINE(PAGE_END_LINE - 1), (uint8_t *)buf, LEFT_MODE);
}