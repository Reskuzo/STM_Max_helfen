/*
 * display_audio.c - H735G-DK adaptation
 */
#include "display_audio.h"
#include "display_theme.h"
#include "stm32h735g_discovery_lcd.h"
#include "../../../Utilities/lcd/stm32_lcd.h"

void Draw_AudioWaveform(int16_t *pData, uint32_t Size)
{
    static uint16_t last_y = WAVE_CENTER_Y;

    /* Clear previous waveform strip */
    BSP_LCD_FillRect(0, PAGE_START_X, WAVE_CENTER_Y - WAVE_AMPLITUDE - 5,
                     LCD_WIDTH, (WAVE_AMPLITUDE * 2) + 10, UTIL_LCD_COLOR_DARKCYAN);

    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
    uint16_t draw_width = LCD_WIDTH - (2 * PAGE_START_X);

    for (uint16_t x = PAGE_START_X; x < LCD_WIDTH - PAGE_START_X; x++)
    {
        uint32_t buf_index = (uint32_t)(x - PAGE_START_X) * Size / draw_width;
        if (buf_index >= Size) buf_index = Size - 1;

        int16_t sample   = pData[buf_index];
        int16_t y_offset = (sample * WAVE_AMPLITUDE) / 32768;
        uint16_t current_y = (uint16_t)(WAVE_CENTER_Y - y_offset);

        if (x > PAGE_START_X) {
            UTIL_LCD_DrawLine(x - 1, last_y, x, current_y, UTIL_LCD_COLOR_WHITE);
        }
        last_y = current_y;
    }
}