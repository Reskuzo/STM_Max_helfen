/*
 * noise-filter.c
 *
 *  Created on: Feb 21, 2026
 *      Author: yannick
 */

#include "noise_filter.h"
#include <string.h>
#include <math.h>

uint16_t   intensity_threshold = 2316;
freq_range frequency_filters[50];

uint16_t noise_filter_enabled = 0b1 /* bit 0 = global enable */    +
    (1 << MEDIAN_FILTER)  +
    (0 << GAUSS_LOWPASS)  +
    (0 << WIENER_FILTER)  +
    (0 << AUD_THRESHOLD)  +
    (0 << FREQNC_WINDOW)  +
    (0 << DCOFF_REMOVAL)  +
    (0 << HANNING_WIN_F)  +
    (0 << TEMPL_FFT_AVG)  +
    (0 << SPECTR_SUBPTR)  +
    (0 << CMSIS_DSP_BIQ)  ;

#define MAX_BUF_SIZE  512
#define FFT_MAX_BINS  256

uint8_t filter_enabled(uint8_t filter_index)
{
    return ((noise_filter_enabled & 1u) != 0) && ((noise_filter_enabled & (1u << filter_index)) != 0);
}

void toggle_filter(uint8_t filter_index)
{
    noise_filter_enabled ^= (1u << filter_index);
}

uint16_t get_noise_filter_flags(void)
{
    return noise_filter_enabled;
}

/* =================================  TIME-DOMAIN FILTERS  ============================= */

/* -- DC offset removal (1st-order IIR high-pass, alpha = 0.995) ----------- */

static float dc_x_prev = 0.0f;
static float dc_y_prev = 0.0f;

void apply_dc_removal(int16_t* s, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        float x  = (float)s[i];
        float y  = x - dc_x_prev + 0.995f * dc_y_prev;
        dc_x_prev = x;
        dc_y_prev = y;
        s[i] = (int16_t)y;
    }
}

/* -- 5-tap median filter --------------------------------------------------- */

static void windowSort(int16_t a[5])
{
    for (int i = 1; i < FILTER_WINDOW_SIZE; i++) {
        int16_t key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
}

static int16_t med_out[MAX_BUF_SIZE];

void apply_median_filter(int16_t* samples, uint32_t num_samples)
{
    uint32_t len = (num_samples > MAX_BUF_SIZE) ? MAX_BUF_SIZE : num_samples;
    for (uint32_t i = 0; i < len; i++) {
        int16_t window[FILTER_WINDOW_SIZE];
        for (int k = FILTER_WINDOW_SIZE / (-2); k <= FILTER_WINDOW_SIZE / (2); k++) {
            int window_offset_index = (int)i + k; // clamp window index to the array
            if (window_offset_index < 0)          window_offset_index = 0;
            if (window_offset_index >= (int)len)  window_offset_index = (int)len - 1;
            window[k + 2] = samples[window_offset_index];
        }
        windowSort(window); // sorts an array of len 5 (must be replaced if window size > 5, but i don't know a better way)
        med_out[i] = window[FILTER_WINDOW_SIZE / (2)];  // middle element = median
    }
    memcpy(samples, med_out, len * sizeof(int16_t));
}

/* -- Gaussian lowpass (5-tap kernel: 1 4 6 4 1 / 16) ---------------------- */

static int16_t gauss_out[MAX_BUF_SIZE];
static const int8_t gauss_kernel[FILTER_WINDOW_SIZE] = {1, 4, 6, 4, 1};

void apply_gaussian_lowpass(int16_t* samples, uint32_t num_samples)
{
    uint32_t len = (num_samples > MAX_BUF_SIZE) ? MAX_BUF_SIZE : num_samples;
    for (uint32_t i = 0; i < len; i++) {
        int32_t acc = 0;
        for (int k = FILTER_WINDOW_SIZE / (-2); k <= FILTER_WINDOW_SIZE / (2); k++) {
            int window_offset_index = (int)i + k; // clamp to fit in the array
            if (window_offset_index < 0)          window_offset_index = 0;
            if (window_offset_index >= (int)len)  window_offset_index = (int)len - 1;
            acc += (int32_t)samples[window_offset_index] * gauss_kernel[k + 2];
        }
        gauss_out[i] = (int16_t)(acc >> 4);  /* divide by 16 */
    }
    memcpy(samples, gauss_out, len * sizeof(int16_t));
}

/* -- Wiener filter (7-tap local window) ----------------------------------- */

#define WIENER_HALF 3   /* window = 2*WIENER_HALF + 1 = 7 samples */

static int16_t wiener_out[MAX_BUF_SIZE];

void apply_wiener_filter(int16_t* s, uint32_t n)
{
    uint32_t len = (n > MAX_BUF_SIZE) ? MAX_BUF_SIZE : n;

    /* Estimate global noise variance from the entire buffer */
    float g_mean = 0.0f;
    for (uint32_t i = 0; i < len; i++) g_mean += s[i];
    g_mean /= len;

    float noise_var = 0.0f;
    for (uint32_t i = 0; i < len; i++) {
        float d = s[i] - g_mean;
        noise_var += d * d;
    }
    noise_var /= len;

    for (uint32_t i = 0; i < len; i++) {
        float lmean = 0.0f;
        int   cnt   = 0;
        for (int k = -WIENER_HALF; k <= WIENER_HALF; k++) {
            int idx = (int)i + k;
            if (idx < 0 || idx >= (int)len) continue;
            lmean += s[idx];
            cnt++;
        }
        lmean /= cnt;

        float lvar = 0.0f;
        for (int k = -WIENER_HALF; k <= WIENER_HALF; k++) {
            int idx = (int)i + k;
            if (idx < 0 || idx >= (int)len) continue;
            float d = s[idx] - lmean;
            lvar += d * d;
        }
        lvar /= cnt;

        /* Wiener gain: suppress signal where local variance ~ noise variance */
        float gain = (lvar <= noise_var) ? 0.0f : (lvar - noise_var) / lvar;
        wiener_out[i] = (int16_t)(lmean + gain * ((float)s[i] - lmean));
    }
    memcpy(s, wiener_out, len * sizeof(int16_t));
}

/* -- Intensity threshold (hard noise gate) --------------------------------- */

void apply_intensity_gate(int16_t* s, uint32_t n)
{
    int16_t thr = (int16_t)intensity_threshold;
    for (uint32_t i = 0; i < n; i++) {
        if (s[i] > -thr && s[i] < thr) s[i] = 0;
    }
}

/* -- Biquad lowpass (Direct-Form II, Butterworth 2nd-order @ fc=1kHz/fs=16kHz) */
/* Coefficients for fc=1000 Hz, fs=16000 Hz, Q=0.7071:                        */
/*   b = [0.0300, 0.0599, 0.0300]   a = [-1.4543, 0.5741]                     */

static float bq_w[2] = {0.0f, 0.0f};

void apply_biquad_lowpass(int16_t* s, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        float w  = (float)s[i] - (-1.4543f) * bq_w[0] - 0.5741f * bq_w[1];
        float y  = 0.0300f * w + 0.0599f * bq_w[0] + 0.0300f * bq_w[1];
        bq_w[1]  = bq_w[0];
        bq_w[0]  = w;
        s[i] = (int16_t)y;
    }
}

/* ===========================  PRE-FFT FILTER  ============================== */

/* -- Hanning window (apply to samples in-place before FFT) ---------------- */

void apply_hanning_window(int16_t* s, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        float w = 0.5f * (1.0f - cosf(2.0f * 3.14159265f * i / (n - 1)));
        s[i] = (int16_t)((float)s[i] * w);
    }
}

/* ===========================  FFT-DOMAIN FILTERS  ========================= */

/* -- Temporal FFT averaging (leaky integrator per bin) -------------------- */

#define TEMPL_ALPHA 0.3f   /* weight of current frame; lower = smoother */

static float fft_avg_state[FFT_MAX_BINS] = {0};

void apply_temporal_avg(float* magnitudes, uint32_t bins)
{
    uint32_t b = (bins > FFT_MAX_BINS) ? FFT_MAX_BINS : bins;
    for (uint32_t i = 0; i < b; i++) {
        fft_avg_state[i] = TEMPL_ALPHA * magnitudes[i]
                         + (1.0f - TEMPL_ALPHA) * fft_avg_state[i];
        magnitudes[i] = fft_avg_state[i];
    }
}

/* -- Spectral subtraction (per-bin noise floor estimation) ---------------- */

/* Noise floor is updated when mean bin energy is below NOISE_ENERGY_THR.    */
#define SPEC_SUB_UPDATE_ALPHA  0.01f
#define NOISE_ENERGY_THR       200.0f

static float noise_floor[FFT_MAX_BINS] = {0};
static uint8_t noise_floor_ready = 0;

void apply_spectral_subtraction(float* magnitudes, uint32_t bins)
{
    uint32_t b = (bins > FFT_MAX_BINS) ? FFT_MAX_BINS : bins;

    /* Seed noise floor on first call */
    if (!noise_floor_ready) {
        for (uint32_t i = 0; i < b; i++) noise_floor[i] = magnitudes[i];
        noise_floor_ready = 1;
        return;
    }

    /* Check mean energy to detect silence */
    float energy = 0.0f;
    for (uint32_t i = 0; i < b; i++) energy += magnitudes[i];

    if ((energy / b) < NOISE_ENERGY_THR) {
        /* Slowly track the noise floor */
        for (uint32_t i = 0; i < b; i++) {
            noise_floor[i] = (1.0f - SPEC_SUB_UPDATE_ALPHA) * noise_floor[i]
                           +          SPEC_SUB_UPDATE_ALPHA  * magnitudes[i];
        }
    }

    /* Subtract and clamp */
    for (uint32_t i = 0; i < b; i++) {
        magnitudes[i] -= noise_floor[i];
        if (magnitudes[i] < 0.0f) magnitudes[i] = 0.0f;
    }
}

/* -- Frequency range gate (zero specific Hz bands) ------------------------ */

static void freq_window_gate(float* magnitudes, uint32_t bins, uint32_t sample_rate)
{
    for (int f = 0; f < 50; f++) {
        if (frequency_filters[f].is_last) break;
        uint32_t b_start = frequency_filters[f].start * bins * 2 / sample_rate;
        uint32_t b_end   = frequency_filters[f].end   * bins * 2 / sample_rate;
        if (b_end >= bins) b_end = bins - 1;
        for (uint32_t bi = b_start; bi <= b_end; bi++) magnitudes[bi] = 0.0f;
    }
}



void apply_filters(int16_t* samples, uint32_t num_samples)
{
    if (!filter_enabled(0)) return;  /* global enable check (bit 0) */
    if (filter_enabled(DCOFF_REMOVAL)) apply_dc_removal(samples, num_samples);
    if (filter_enabled(MEDIAN_FILTER)) apply_median_filter(samples, num_samples);
    if (filter_enabled(GAUSS_LOWPASS)) apply_gaussian_lowpass(samples, num_samples);
    if (filter_enabled(WIENER_FILTER)) apply_wiener_filter(samples, num_samples);
    if (filter_enabled(AUD_THRESHOLD)) apply_intensity_gate(samples, num_samples);
    if (filter_enabled(CMSIS_DSP_BIQ)) apply_biquad_lowpass(samples, num_samples);
}

void apply_fft_filters(float* magnitudes, uint32_t bins, uint32_t sample_rate)
{
    if (!filter_enabled(0)) return;
    if (filter_enabled(TEMPL_FFT_AVG)) apply_temporal_avg(magnitudes, bins);
    if (filter_enabled(SPECTR_SUBPTR)) apply_spectral_subtraction(magnitudes, bins);
    if (filter_enabled(FREQNC_WINDOW)) freq_window_gate(magnitudes, bins, sample_rate);
}
