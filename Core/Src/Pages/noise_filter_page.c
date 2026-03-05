/*
 * noise_filter_page.c
 *
 *  Created on: Feb 24, 2026
 *      Author: yannick
 */

#include "noise_filter_page.h"
#include "../Audio/noise_filter.h"
#include "../Display/display_main.h"
#include "../Display/display_theme.h"
#include "../../../Utilities/lcd/stm32_lcd.h"
#include <stdio.h>

/* ── Filter list ─────────────────────────────────────────────────────────── */

#define NUM_FILTERS 11

static const uint8_t filter_bit[NUM_FILTERS] = {
    0,               /* global enable (master) */
    DCOFF_REMOVAL,
    MEDIAN_FILTER,
    GAUSS_LOWPASS,
    WIENER_FILTER,
    AUD_THRESHOLD,
    CMSIS_DSP_BIQ,
    HANNING_WIN_F,
    TEMPL_FFT_AVG,
    SPECTR_SUBPTR,
    FREQNC_WINDOW,
};

static const char* filter_name[NUM_FILTERS] = {
    "Global Enable (MASTER)",
    "DC Offset Removal",
    "Median Filter",
    "Gaussian Lowpass",
    "Wiener Filter",
    "Noise Gate (Threshold)",
    "Biquad Lowpass",
    "Hanning Window (pre-FFT)",
    "FFT Temporal Averaging",
    "Spectral Subtraction",
    "Frequency Window Gate",
};

/* ── State ───────────────────────────────────────────────────────────────── */

static uint8_t cursor = 0;
static uint8_t needs_redraw = 1;

/* ── Drawing ─────────────────────────────────────────────────────────────── */

#define LIST_START_LINE  6   /* first display line used for the filter list */

static void draw_filter_list(void)
{
    uint16_t flags = get_noise_filter_flags();

    for (uint8_t i = 0; i < NUM_FILTERS; i++) {
        uint8_t bit    = filter_bit[i];
        uint8_t is_on  = (flags & (1u << bit)) ? 1 : 0;
        uint8_t sel    = (i == cursor);

        char line_buf[60];
        sprintf(line_buf, "%s [%c] %s",
                sel ? ">" : " ",
                is_on ? 'X' : ' ',
                filter_name[i]);

        println(LIST_START_LINE + i, line_buf);
    }
}

/* ── Page callbacks ───────────────────────────────────────────────────────── */

void noise_filter_page_init(void)
{
    cursor       = 0;
    needs_redraw = 1;
    display_default_page();
    draw_filter_list();
}

void noise_filter_page_onTick(void)
{
    if (!needs_redraw) return;
    draw_filter_list();
    needs_redraw = 0;
}

void noise_filter_page_cursor_up(void)
{
    if (cursor > 0) cursor--;
    needs_redraw = 1;
}

void noise_filter_page_cursor_down(void)
{
    if (cursor < NUM_FILTERS - 1) cursor++;
    needs_redraw = 1;
}

void noise_filter_page_toggle(void)
{
    toggle_filter(filter_bit[cursor]);
    needs_redraw = 1;
}
