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
    FILTER_MUTE,          /* Silencio total */
    FILTER_VOLUME_HIGH,   /* Volumen x3 (muy fuerte) */
    FILTER_DISTORTION,    /* Distorsión extrema */
} filter_type_t;

/**
 * @brief Filter configuration
 */
typedef struct {
    filter_type_t type;

    /* FIR specific */
    int num_taps;       /* Number of FIR taps (odd number, 31-255) */

    /* System */
    float sample_rate;  /* Sample rate (Hz) */
} filter_config_t;

/**
 * @brief Generic audio filter
 */
typedef struct {
    filter_config_t config;

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
void audio_filter_process_buffer(audio_filter_t *filter, const int16_t *input, int16_t *output, uint32_t num_samples);

/**
 * @brief Enable/disable filter
 *
 * @param filter Pointer to audio_filter_t structure
 * @param enable 1 to enable, 0 to disable (bypass)
 */
void audio_filter_enable(audio_filter_t *filter, int enable);

/**
 * @brief Print filter information
 *
 * @param filter Pointer to audio_filter_t structure
 */
void audio_filter_print_info(audio_filter_t *filter);

#endif /* AUDIO_FILTER_H */
