/*
 * noise_filter.h
 *
 *  Created on: Feb 21, 2026
 *      Author: yannick
 */

#ifndef SRC_AUDIO_NOISE_FILTER_H_
#define SRC_AUDIO_NOISE_FILTER_H_

#include <stdint.h>

#define true  1
#define false 0

/* ── Filter indices (bit positions in nf_config) ─────────────────────────── */
/*    Bit 0 is the global enable flag                                          */
#define MEDIAN_FILTER 1   /* time-domain: 5-tap median                        */
#define GAUSS_LOWPASS 2   /* time-domain: 5-tap Gaussian FIR                  */
#define WIENER_FILTER 3   /* time-domain: local Wiener denoising               */
#define AUD_THRESHOLD 4   /* time-domain: hard noise gate                      */
#define FREQNC_WINDOW 6   /* freq-domain: set selected Hz bands to 0         */
#define DCOFF_REMOVAL 5   /* time-domain: 1st-order IIR high-pass (DC remove)  */
#define HANNING_WIN_F 7   /* pre-FFT:     Hanning window on input samples      */
#define TEMPL_FFT_AVG 8   /* freq-domain: temporal leaky-integrator averaging  */
#define SPECTR_SUBPTR 9   /* freq-domain: spectral subtraction noise floor     */
#define CMSIS_DSP_BIQ 10  /* time-domain: 2nd-order IIR biquad lowpass         */

extern uint16_t noise_filter_enabled;



/* ── Interactive filter parameters ──────────────────────────────────────── */
typedef struct {
    uint32_t start;   /* Hz */
    uint32_t end;     /* Hz */
    uint8_t  is_last; /* sentinel: set 1 on the last active entry */
} freq_range;

extern uint16_t   intensity_threshold;
extern freq_range frequency_filters[50];


/* ---- Filter settings ------------------------------------------------------ */
#define FILTER_WINDOW_SIZE   5


/* ── Function declarations ───────────────────────────────────────────────── */

/** @brief Returns 1 if the global enable (bit 0) and the given filter bit are both set. */
uint8_t filter_enabled(uint8_t filter_index);

/** @brief Toggle the bit for the given filter index in noise_filter_enabled. */
void toggle_filter(uint8_t filter_index);

/** @brief Return the raw noise_filter_enabled bitmask (for display purposes). */
uint16_t get_noise_filter_flags(void);

/**
 * @brief Apply all enabled filters in-place.
 *        Order: DC removal -> median -> Gaussian -> Wiener -> noise gate -> biquad.
 */
void apply_filters(int16_t* samples, uint32_t num_samples);

/**
 * @brief Apply all enabled FFT-domain filters in-place to a magnitude array.
 * @param magnitudes  Positive-frequency magnitude bins (float)
 * @param bins        Number of bins (FFT_SIZE / 2)
 * @param sample_rate Sample rate in Hz used to convert Hz to bin index
 */
void apply_fft_filters(float* magnitudes, uint32_t bins, uint32_t sample_rate);

/* ── Individually callable filters (time-domain) ────────────────────────── */
void apply_dc_removal(int16_t* samples, uint32_t num_samples);
void apply_median_filter(int16_t* samples, uint32_t num_samples);
void apply_gaussian_lowpass(int16_t* samples, uint32_t num_samples);
void apply_wiener_filter(int16_t* samples, uint32_t num_samples);
void apply_intensity_gate(int16_t* samples, uint32_t num_samples);
void apply_biquad_lowpass(int16_t* samples, uint32_t num_samples);

/* ── Individually callable filters (FFT-domain / pre-FFT) ───────────────── */
/** @brief Multiply samples by a Hanning window before FFT (modifies buffer in-place). */
void apply_hanning_window(int16_t* samples, uint32_t num_samples);
/** @brief Temporal leaky-integrator smoothing across FFT frames. */
void apply_temporal_avg(float* magnitudes, uint32_t bins);
/** @brief Spectral subtraction: estimate and subtract per-bin noise floor. */
void apply_spectral_subtraction(float* magnitudes, uint32_t bins);

#endif /* SRC_AUDIO_NOISE_FILTER_H_ */
