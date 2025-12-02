/**
 * @file audio_filter.h
 * @brief Digital Audio Filters (FIR & IIR) for real-time processing
 *
 * Implementa filtros digitales en tiempo real:
 * - FIR (Finite Impulse Response): Fase lineal, siempre estables
 * - IIR (Infinite Impulse Response): Más eficientes, menos coeficientes
 *
 * Tipos de filtros:
 * - Low-pass (paso-bajo): Deja pasar bajas frecuencias
 * - High-pass (paso-alto): Deja pasar altas frecuencias
 * - Band-pass (paso-banda): Deja pasar rango de frecuencias
 * - Band-stop (notch): Elimina rango de frecuencias
 */

#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#include <stdint.h>

/* ============================================================================
 * FILTER TYPES & PARAMETERS
 * ============================================================================ */

/**
 * @brief Filter type enumeration
 */
typedef enum {
    FILTER_NONE = 0,
    FILTER_LOWPASS,     /* Paso-bajo: fc = cutoff frequency */
    FILTER_HIGHPASS,    /* Paso-alto: fc = cutoff frequency */
    FILTER_BANDPASS,    /* Paso-banda: fc1, fc2 */
    FILTER_BANDSTOP,    /* Notch: fc1, fc2 */
} filter_type_t;

/**
 * @brief Filter implementation
 */
typedef enum {
    FILTER_IIR_BUTTERWORTH,  /* IIR Butterworth (flat response) */
    FILTER_IIR_CHEBYSHEV,    /* IIR Chebyshev Type I (sharper cutoff, ripple) */
    FILTER_FIR_WINDOW,       /* FIR windowed (linear phase) */
} filter_impl_t;

/**
 * @brief Filter configuration
 */
typedef struct {
    filter_type_t type;
    filter_impl_t implementation;

    /* Frequency parameters (Hz) */
    float fc;           /* Cutoff frequency (lowpass/highpass) */
    float fc1, fc2;     /* Band frequencies (bandpass/bandstop) */

    /* IIR specific */
    int order;          /* Filter order (1-8 for IIR) */
    float ripple_db;    /* Chebyshev ripple (dB) */

    /* FIR specific */
    int num_taps;       /* Number of FIR taps (odd number, 31-255) */

    /* System */
    float sample_rate;  /* Sample rate (Hz) */
} filter_config_t;

/* ============================================================================
 * FILTER STATE STRUCTURES
 * ============================================================================ */

#define MAX_FILTER_ORDER 8
#define MAX_FIR_TAPS 255
#define MAX_BIQUAD_SECTIONS 4  /* For cascaded biquads */

/**
 * @brief Biquad section (2nd order IIR)
 *
 * Transfer function:
 *        b0 + b1*z^-1 + b2*z^-2
 * H(z) = ------------------------
 *        1 + a1*z^-1 + a2*z^-2
 */
typedef struct {
    /* Coefficients */
    float b0, b1, b2;  /* Numerator (feedforward) */
    float a1, a2;      /* Denominator (feedback) - a0 normalized to 1 */

    /* State variables (Direct Form II Transposed) */
    float z1, z2;      /* Delay elements */
} biquad_t;

/**
 * @brief IIR Filter (cascaded biquads)
 */
typedef struct {
    int num_sections;
    biquad_t sections[MAX_BIQUAD_SECTIONS];
    float gain;        /* Overall gain */
} iir_filter_t;

/**
 * @brief FIR Filter
 */
typedef struct {
    int num_taps;
    float coeffs[MAX_FIR_TAPS];
    float buffer[MAX_FIR_TAPS];  /* Circular buffer for delay line */
    int buffer_index;
} fir_filter_t;

/**
 * @brief Generic audio filter
 */
typedef struct {
    filter_config_t config;

    /* Implementation */
    union {
        iir_filter_t iir;
        fir_filter_t fir;
    } impl;

    int enabled;
    uint64_t samples_processed;
} audio_filter_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize audio filter with configuration
 *
 * @param filter Pointer to audio_filter_t structure
 * @param config Pointer to filter configuration
 * @return 0 on success, -1 on error
 */
int audio_filter_init(audio_filter_t *filter, const filter_config_t *config);

/**
 * @brief Process single sample through filter
 *
 * @param filter Pointer to audio_filter_t structure
 * @param input Input sample
 * @return Filtered output sample
 */
int16_t audio_filter_process_sample(audio_filter_t *filter, int16_t input);

/**
 * @brief Process buffer of samples
 *
 * @param filter Pointer to audio_filter_t structure
 * @param input Input buffer
 * @param output Output buffer
 * @param num_samples Number of samples to process
 */
void audio_filter_process_buffer(audio_filter_t *filter,
                                  const int16_t *input,
                                  int16_t *output,
                                  uint32_t num_samples);

/**
 * @brief Reset filter state (clear delay lines)
 *
 * @param filter Pointer to audio_filter_t structure
 */
void audio_filter_reset(audio_filter_t *filter);

/**
 * @brief Enable/disable filter
 *
 * @param filter Pointer to audio_filter_t structure
 * @param enable 1 to enable, 0 to disable (bypass)
 */
void audio_filter_enable(audio_filter_t *filter, int enable);

/**
 * @brief Get filter frequency response at given frequency
 *
 * @param filter Pointer to audio_filter_t structure
 * @param frequency Frequency to evaluate (Hz)
 * @return Magnitude response (linear, not dB)
 */
float audio_filter_get_response(audio_filter_t *filter, float frequency);

/**
 * @brief Print filter information
 *
 * @param filter Pointer to audio_filter_t structure
 */
void audio_filter_print_info(audio_filter_t *filter);

/* ============================================================================
 * PRESET CONFIGURATIONS
 * ============================================================================ */

/**
 * @brief Create lowpass filter configuration
 */
filter_config_t audio_filter_config_lowpass(float fc, int order, float fs);

/**
 * @brief Create highpass filter configuration
 */
filter_config_t audio_filter_config_highpass(float fc, int order, float fs);

/**
 * @brief Create bandpass filter configuration
 */
filter_config_t audio_filter_config_bandpass(float fc1, float fc2, int order, float fs);

/**
 * @brief Create bandstop (notch) filter configuration
 */
filter_config_t audio_filter_config_bandstop(float fc1, float fc2, int order, float fs);

#endif /* AUDIO_FILTER_H */
