/*
 * fourier_page.c
 *
 *  Created on: Feb 20, 2026
 *      Author: yannick
 */

#include "fourier_page.h"
#include "../Audio/audio_main.h"
#include "../Display/display_main.h"
#include "../Display/display_theme.h"
#include "stm32h735g_discovery_lcd.h"
#include "../../../Utilities/lcd/stm32_lcd.h"
#include "../../../Utilities/Fonts/fonts.h"
#include <math.h>
#include <stdio.h>

/* ── Constants ────────────────────────────────────────────────────────────── */

#define FFT_SIZE    512
#define FFT_BINS    (FFT_SIZE / 2)      /* 256 positive-frequency bins       */
#define SAMPLE_RATE 16000               /* Hz                                */

/* Spectrum draw area (inside the white rect from display_default_page)
 * Left margin leaves room for Y-axis labels; bottom margin for X-axis labels */
#define SPEC_X0     35
#define SPEC_Y0     50
#define SPEC_X1     472
#define SPEC_Y1     230
#define SPEC_W      (SPEC_X1 - SPEC_X0)  /* 720 px                          */
#define SPEC_H      (SPEC_Y1 - SPEC_Y0)  /* 328 px                          */

/* ── State ────────────────────────────────────────────────────────────────── */

uint16_t freq_cursor = 1;
uint8_t  fourier_hold  = 0;   /* 1 = freeze display                         */
static uint8_t log_scale = 0; /* 0 = linear, 1 = logarithmic                */



/* ── Axes ─────────────────────────────────────────────────────────────────── */

static void draw_axes(void)
{
    UTIL_LCD_SetFont(&Font12);
    UTIL_LCD_SetTextColor(FOREGROUND);

    /* X-axis line */
    UTIL_LCD_DrawLine(SPEC_X0, SPEC_Y1, SPEC_X1, SPEC_Y1, FOREGROUND);
    /* Y-axis line */
    UTIL_LCD_DrawLine(SPEC_X0, SPEC_Y0, SPEC_X0, SPEC_Y1, FOREGROUND);

    /* X-axis: tick every 1 kHz (0 Hz to 8 kHz = 9 ticks) */
    for (int khz = 0; khz <= 8; khz++) {
        uint32_t bin = (uint32_t)khz * 1000 * FFT_SIZE / SAMPLE_RATE; /* bin index */
        uint16_t x   = SPEC_X0 + (uint16_t)(bin * SPEC_W / FFT_BINS);

        /* tick mark */
        UTIL_LCD_DrawLine(x, SPEC_Y1, x, SPEC_Y1 + 4, FOREGROUND);

        /* label (centered on tick) */
        char lbl[6];
        if (khz == 0)
            sprintf(lbl, "0");
        else
            sprintf(lbl, "%dk", khz);

        /* each Font12 char is ~7 px wide; offset by half label width */
        uint16_t lbl_x = (x > 7) ? (x - 7) : 0;
        UTIL_LCD_DisplayStringAt(lbl_x, SPEC_Y1 + 6, (uint8_t *)lbl, LEFT_MODE);
    }

    /* Y-axis: 5 ticks at 0 %, 25 %, 50 %, 75 %, 100 % of full scale */
    for (int t = 0; t <= 4; t++) {
        uint16_t y = SPEC_Y1 - (uint16_t)(t * SPEC_H / 4);

        /* tick mark */
        UTIL_LCD_DrawLine(SPEC_X0 - 4, y, SPEC_X0, y, FOREGROUND);

        /* label right-aligned just left of the axis */
        char lbl[8];
        if (log_scale > 0)
            sprintf(lbl, "%2ddB", t * 20);   /* 0, 20, 40, 60, 80 dB */
        else
            sprintf(lbl, "%3d%%", t * 25);   /* 0, 25, 50, 75, 100 % */

        /* right-align: 4 chars × 7 px = 28 px; leave 2 px gap before tick */
        uint16_t lbl_x = (SPEC_X0 > 32) ? (SPEC_X0 - 32) : 0;
        UTIL_LCD_DisplayStringAt(lbl_x, (y > 6) ? (y - 6) : 0, (uint8_t *)lbl, LEFT_MODE);
    }

    UTIL_LCD_SetFont(&Font16); /* restore default */
}



static void draw_spectrum(void)
{
    /* Find max magnitude for normalisation */
    float max_mag = 1.0f;
    for (int i = 1; i < FFT_BINS; i++) {
        if (FrequencyBuffer[i] > max_mag)
            max_mag = FrequencyBuffer[i];
    }

    /* Clear spectrum area */
    BSP_LCD_FillRect(0, SPEC_X0, SPEC_Y0, SPEC_W, SPEC_H, BACKGROUND);

    /* all mag > 0 =>  Draw one vertical bar per FFT bin (bins 1..FFT_BINS-1) */
    for (int i = 1; i < FFT_BINS; i++) {
        uint16_t x = SPEC_X0 + (uint16_t)((i * SPEC_W) / FFT_BINS);

        float norm;
        if (log_scale > 0) {
            /* Map magnitude to dB, clamp to [0, 100] dB range */
            float db = 20.0f * log10f(FrequencyBuffer[i]  + 1e-6f) + 100.0f;
            if (db < 0.0f) db = 0.0f;
            norm = db / 100.0f;
        } else {
            norm = FrequencyBuffer[i] / max_mag;
        }

        uint16_t bar_h = (uint16_t)(norm * SPEC_H);
        if (bar_h == 0) continue;

        uint32_t color = (i == freq_cursor) ? UTIL_LCD_COLOR_DARKYELLOW : FOREGROUND;
        BSP_LCD_FillRect(0, x, SPEC_Y1 - bar_h, 2, bar_h, color);
    }

    draw_axes();
}

/* ── Page callbacks ───────────────────────────────────────────────────────── */

void fourier_init(void)
{
    display_default_page();

	audio_transforms =
			(1 << DO_PDM_PCM_TRANSFORM) +
			(1 << DO_FOURIER_TRANSFORM) +
			(0 << DO_INVERSE_TRANSFORM) ;
    start_recording();


    fourier_hold = 0;
    freq_cursor  = 1;
}

void fourier_onTick(void)
{
    if (!audio_ready) return;
    process_audio();
    audio_ready = 0;

    if (fourier_hold) return; /* frozen — consume the flag but don't redraw */

    process_audio();
    draw_spectrum();
}

void fourier_cursor_left(void)
{
    if (freq_cursor > 1)
        freq_cursor--;
}

void fourier_cursor_right(void)
{
    if (freq_cursor < FFT_BINS - 1)
        freq_cursor++;
}

void fourier_toggle_hold(void)
{
    fourier_hold = !fourier_hold;
}

void fourier_toggle_scale(void)
{
    log_scale = (log_scale + 1) % 4;
}
