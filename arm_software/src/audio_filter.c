/**
 * @file audio_filter.c
 * @brief Digital Audio Filters Implementation
 */

#include "audio_filter.h"
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ============================================================================
 * BIQUAD DESIGN (IIR 2nd Order Sections)
 * ============================================================================ */

/**
 * @brief Design 2nd order Butterworth lowpass biquad
 *
 * Based on Robert Bristow-Johnson's Audio EQ Cookbook
 */
static void biquad_design_lowpass(biquad_t *bq, float fc, float fs, float Q) {
    float w0 = 2.0f * M_PI * fc / fs;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / (2.0f * Q);

    float b0 = (1.0f - cos_w0) / 2.0f;
    float b1 = 1.0f - cos_w0;
    float b2 = (1.0f - cos_w0) / 2.0f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_w0;
    float a2 = 1.0f - alpha;

    /* Normalize by a0 */
    bq->b0 = b0 / a0;
    bq->b1 = b1 / a0;
    bq->b2 = b2 / a0;
    bq->a1 = a1 / a0;
    bq->a2 = a2 / a0;

    /* Initialize state */
    bq->z1 = 0.0f;
    bq->z2 = 0.0f;
}

/**
 * @brief Design 2nd order Butterworth highpass biquad
 */
static void biquad_design_highpass(biquad_t *bq, float fc, float fs, float Q) {
    float w0 = 2.0f * M_PI * fc / fs;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / (2.0f * Q);

    float b0 = (1.0f + cos_w0) / 2.0f;
    float b1 = -(1.0f + cos_w0);
    float b2 = (1.0f + cos_w0) / 2.0f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_w0;
    float a2 = 1.0f - alpha;

    /* Normalize by a0 */
    bq->b0 = b0 / a0;
    bq->b1 = b1 / a0;
    bq->b2 = b2 / a0;
    bq->a1 = a1 / a0;
    bq->a2 = a2 / a0;

    /* Initialize state */
    bq->z1 = 0.0f;
    bq->z2 = 0.0f;
}

/**
 * @brief Design 2nd order Butterworth bandpass biquad
 */
static void biquad_design_bandpass(biquad_t *bq, float fc, float fs, float Q) {
    float w0 = 2.0f * M_PI * fc / fs;
    float cos_w0 = cosf(w0);
    float sin_w0 = sinf(w0);
    float alpha = sin_w0 / (2.0f * Q);

    float b0 = alpha;
    float b1 = 0.0f;
    float b2 = -alpha;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_w0;
    float a2 = 1.0f - alpha;

    /* Normalize by a0 */
    bq->b0 = b0 / a0;
    bq->b1 = b1 / a0;
    bq->b2 = b2 / a0;
    bq->a1 = a1 / a0;
    bq->a2 = a2 / a0;

    /* Initialize state */
    bq->z1 = 0.0f;
    bq->z2 = 0.0f;
}

/**
 * @brief Process sample through biquad (Direct Form II Transposed)
 *
 * Most efficient structure for fixed-point and floating-point
 */
static inline float biquad_process(biquad_t *bq, float input) {
    float output = bq->b0 * input + bq->z1;
    bq->z1 = bq->b1 * input - bq->a1 * output + bq->z2;
    bq->z2 = bq->b2 * input - bq->a2 * output;
    return output;
}

/* ============================================================================
 * IIR FILTER INITIALIZATION
 * ============================================================================ */

/**
 * @brief Get Butterworth Q factor for cascaded biquads
 *
 * For order N filter, we need N/2 biquad sections (round up)
 * Each section has specific Q to achieve Butterworth response
 */
static float butterworth_q_factor(int section, int total_sections) {
    /* Q = 1 / (2 * cos((2*k + 1) * pi / (4*N))) where k = section number */
    float angle = M_PI * (2.0f * section + 1.0f) / (4.0f * total_sections);
    return 1.0f / (2.0f * cosf(angle));
}

/**
 * @brief Initialize IIR lowpass filter
 */
static int iir_init_lowpass(iir_filter_t *iir, float fc, int order, float fs) {
    /* Butterworth requires even order, round up */
    int effective_order = (order + 1) / 2 * 2;
    iir->num_sections = effective_order / 2;

    if (iir->num_sections > MAX_BIQUAD_SECTIONS) {
        ERROR_PRINT("IIR order too high: %d (max %d sections)", order, MAX_BIQUAD_SECTIONS);
        return -1;
    }

    /* Design cascaded biquads */
    for (int i = 0; i < iir->num_sections; i++) {
        float Q = butterworth_q_factor(i, iir->num_sections);
        biquad_design_lowpass(&iir->sections[i], fc, fs, Q);
    }

    iir->gain = 1.0f;
    return 0;
}

/**
 * @brief Initialize IIR highpass filter
 */
static int iir_init_highpass(iir_filter_t *iir, float fc, int order, float fs) {
    int effective_order = (order + 1) / 2 * 2;
    iir->num_sections = effective_order / 2;

    if (iir->num_sections > MAX_BIQUAD_SECTIONS) {
        ERROR_PRINT("IIR order too high: %d", order);
        return -1;
    }

    for (int i = 0; i < iir->num_sections; i++) {
        float Q = butterworth_q_factor(i, iir->num_sections);
        biquad_design_highpass(&iir->sections[i], fc, fs, Q);
    }

    iir->gain = 1.0f;
    return 0;
}

/**
 * @brief Initialize IIR bandpass filter
 */
static int iir_init_bandpass(iir_filter_t *iir, float fc1, float fc2, int order, float fs) {
    /* Cascade lowpass and highpass */
    /* For simplicity, use 1 lowpass + 1 highpass biquad */
    if (order > 2) {
        ERROR_PRINT("Bandpass order > 2 not implemented yet");
        return -1;
    }

    iir->num_sections = 2;

    /* Center frequency and bandwidth */
    float fc_center = sqrtf(fc1 * fc2);
    float bandwidth = fc2 - fc1;
    float Q = fc_center / bandwidth;

    /* Design bandpass biquad */
    biquad_design_bandpass(&iir->sections[0], fc_center, fs, Q);

    /* Second section for higher order (optional) */
    if (order == 2) {
        biquad_design_bandpass(&iir->sections[1], fc_center, fs, Q);
    } else {
        iir->num_sections = 1;
    }

    iir->gain = 1.0f;
    return 0;
}

/* ============================================================================
 * FIR FILTER INITIALIZATION (WINDOWED METHOD)
 * ============================================================================ */

/**
 * @brief Hamming window function
 */
static inline float hamming_window(int n, int N) {
    return 0.54f - 0.46f * cosf(2.0f * M_PI * n / (N - 1));
}

/**
 * @brief Initialize FIR lowpass filter using windowed sinc method
 */
static int fir_init_lowpass(fir_filter_t *fir, float fc, int num_taps, float fs) {
    if (num_taps > MAX_FIR_TAPS) {
        ERROR_PRINT("FIR taps too high: %d (max %d)", num_taps, MAX_FIR_TAPS);
        return -1;
    }

    /* Ensure odd number of taps (symmetric around center) */
    if (num_taps % 2 == 0) {
        num_taps++;
    }

    fir->num_taps = num_taps;
    fir->buffer_index = 0;

    /* Clear buffer */
    memset(fir->buffer, 0, sizeof(fir->buffer));

    /* Design using windowed sinc */
    float fc_norm = fc / fs;  /* Normalized cutoff (0-0.5) */
    int M = (num_taps - 1) / 2;  /* Half-length */

    for (int n = 0; n < num_taps; n++) {
        int i = n - M;  /* Center around 0 */

        /* Ideal sinc lowpass */
        float h;
        if (i == 0) {
            h = 2.0f * fc_norm;
        } else {
            h = sinf(2.0f * M_PI * fc_norm * i) / (M_PI * i);
        }

        /* Apply Hamming window */
        h *= hamming_window(n, num_taps);

        fir->coeffs[n] = h;
    }

    /* Normalize to unity gain at DC */
    float sum = 0.0f;
    for (int n = 0; n < num_taps; n++) {
        sum += fir->coeffs[n];
    }
    for (int n = 0; n < num_taps; n++) {
        fir->coeffs[n] /= sum;
    }

    return 0;
}

/**
 * @brief Initialize FIR highpass filter
 */
static int fir_init_highpass(fir_filter_t *fir, float fc, int num_taps, float fs) {
    /* Design lowpass first */
    if (fir_init_lowpass(fir, fc, num_taps, fs) != 0) {
        return -1;
    }

    /* Spectral inversion: negate all coefficients and add 1 to center */
    int M = (num_taps - 1) / 2;
    for (int n = 0; n < num_taps; n++) {
        fir->coeffs[n] = -fir->coeffs[n];
    }
    fir->coeffs[M] += 1.0f;  /* Add impulse at center */

    return 0;
}

/**
 * @brief Process sample through FIR filter
 */
static inline float fir_process(fir_filter_t *fir, float input) {
    /* Insert new sample into circular buffer */
    fir->buffer[fir->buffer_index] = input;

    /* Convolution (multiply-accumulate) */
    float output = 0.0f;
    int idx = fir->buffer_index;

    for (int i = 0; i < fir->num_taps; i++) {
        output += fir->coeffs[i] * fir->buffer[idx];
        idx = (idx == 0) ? (fir->num_taps - 1) : (idx - 1);
    }

    /* Update buffer index */
    fir->buffer_index = (fir->buffer_index + 1) % fir->num_taps;

    return output;
}

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

int audio_filter_init(audio_filter_t *filter, const filter_config_t *config) {
    if (!filter || !config) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(filter, 0, sizeof(audio_filter_t));
    memcpy(&filter->config, config, sizeof(filter_config_t));

    int result = 0;

    /* Initialize based on implementation type */
    if (config->implementation == FILTER_IIR_BUTTERWORTH) {
        switch (config->type) {
            case FILTER_LOWPASS:
                result = iir_init_lowpass(&filter->impl.iir, config->fc,
                                          config->order, config->sample_rate);
                break;

            case FILTER_HIGHPASS:
                result = iir_init_highpass(&filter->impl.iir, config->fc,
                                           config->order, config->sample_rate);
                break;

            case FILTER_BANDPASS:
                result = iir_init_bandpass(&filter->impl.iir, config->fc1, config->fc2,
                                           config->order, config->sample_rate);
                break;

            default:
                ERROR_PRINT("Unsupported IIR filter type: %d", config->type);
                return -1;
        }
    } else if (config->implementation == FILTER_FIR_WINDOW) {
        switch (config->type) {
            case FILTER_LOWPASS:
                result = fir_init_lowpass(&filter->impl.fir, config->fc,
                                          config->num_taps, config->sample_rate);
                break;

            case FILTER_HIGHPASS:
                result = fir_init_highpass(&filter->impl.fir, config->fc,
                                           config->num_taps, config->sample_rate);
                break;

            default:
                ERROR_PRINT("Unsupported FIR filter type: %d", config->type);
                return -1;
        }
    } else {
        ERROR_PRINT("Unsupported filter implementation: %d", config->implementation);
        return -1;
    }

    if (result == 0) {
        filter->enabled = 1;
        INFO_PRINT("Audio filter initialized successfully");
    }

    return result;
}

int16_t audio_filter_process_sample(audio_filter_t *filter, int16_t input) {
    if (!filter || !filter->enabled) {
        return input;  /* Bypass */
    }

    /* Convert to float (-1.0 to 1.0 range) */
    float x = (float)input / 32768.0f;
    float y = x;

    /* Process through filter */
    if (filter->config.implementation == FILTER_IIR_BUTTERWORTH ||
        filter->config.implementation == FILTER_IIR_CHEBYSHEV) {

        /* Cascade through all biquad sections */
        for (int i = 0; i < filter->impl.iir.num_sections; i++) {
            y = biquad_process(&filter->impl.iir.sections[i], y);
        }
        y *= filter->impl.iir.gain;

    } else if (filter->config.implementation == FILTER_FIR_WINDOW) {
        y = fir_process(&filter->impl.fir, y);
    }

    /* Convert back to int16_t with saturation */
    y *= 32768.0f;
    if (y > 32767.0f) y = 32767.0f;
    if (y < -32768.0f) y = -32768.0f;

    filter->samples_processed++;
    return (int16_t)y;
}

void audio_filter_process_buffer(audio_filter_t *filter,
                                  const int16_t *input,
                                  int16_t *output,
                                  uint32_t num_samples) {
    if (!filter) return;

    for (uint32_t i = 0; i < num_samples; i++) {
        output[i] = audio_filter_process_sample(filter, input[i]);
    }
}

void audio_filter_reset(audio_filter_t *filter) {
    if (!filter) return;

    if (filter->config.implementation == FILTER_IIR_BUTTERWORTH ||
        filter->config.implementation == FILTER_IIR_CHEBYSHEV) {

        for (int i = 0; i < filter->impl.iir.num_sections; i++) {
            filter->impl.iir.sections[i].z1 = 0.0f;
            filter->impl.iir.sections[i].z2 = 0.0f;
        }

    } else if (filter->config.implementation == FILTER_FIR_WINDOW) {
        memset(filter->impl.fir.buffer, 0, sizeof(filter->impl.fir.buffer));
        filter->impl.fir.buffer_index = 0;
    }

    filter->samples_processed = 0;
    INFO_PRINT("Filter state reset");
}

void audio_filter_enable(audio_filter_t *filter, int enable) {
    if (!filter) return;
    filter->enabled = enable;
}

void audio_filter_print_info(audio_filter_t *filter) {
    if (!filter) return;

    printf("========================================\n");
    printf("Audio Filter Information\n");
    printf("========================================\n");

    /* Filter type */
    const char *type_str[] = {"None", "Lowpass", "Highpass", "Bandpass", "Bandstop"};
    printf("Type:            %s\n", type_str[filter->config.type]);

    /* Implementation */
    const char *impl_str[] = {"IIR Butterworth", "IIR Chebyshev", "FIR Window"};
    printf("Implementation:  %s\n", impl_str[filter->config.implementation]);

    /* Parameters */
    printf("Sample Rate:     %.0f Hz\n", filter->config.sample_rate);

    if (filter->config.type == FILTER_BANDPASS || filter->config.type == FILTER_BANDSTOP) {
        printf("Frequency Range: %.1f Hz - %.1f Hz\n", filter->config.fc1, filter->config.fc2);
    } else {
        printf("Cutoff Freq:     %.1f Hz\n", filter->config.fc);
    }

    if (filter->config.implementation == FILTER_FIR_WINDOW) {
        printf("FIR Taps:        %d\n", filter->impl.fir.num_taps);
    } else {
        printf("IIR Order:       %d\n", filter->config.order);
        printf("Biquad Sections: %d\n", filter->impl.iir.num_sections);
    }

    printf("Status:          %s\n", filter->enabled ? "Enabled" : "Disabled");
    printf("Samples Proc:    %llu\n", (unsigned long long)filter->samples_processed);
    printf("========================================\n");
}

/* ============================================================================
 * PRESET CONFIGURATIONS
 * ============================================================================ */

filter_config_t audio_filter_config_lowpass(float fc, int order, float fs) {
    filter_config_t config = {0};
    config.type = FILTER_LOWPASS;
    config.implementation = FILTER_IIR_BUTTERWORTH;
    config.fc = fc;
    config.order = order;
    config.sample_rate = fs;
    return config;
}

filter_config_t audio_filter_config_highpass(float fc, int order, float fs) {
    filter_config_t config = {0};
    config.type = FILTER_HIGHPASS;
    config.implementation = FILTER_IIR_BUTTERWORTH;
    config.fc = fc;
    config.order = order;
    config.sample_rate = fs;
    return config;
}

filter_config_t audio_filter_config_bandpass(float fc1, float fc2, int order, float fs) {
    filter_config_t config = {0};
    config.type = FILTER_BANDPASS;
    config.implementation = FILTER_IIR_BUTTERWORTH;
    config.fc1 = fc1;
    config.fc2 = fc2;
    config.order = order;
    config.sample_rate = fs;
    return config;
}

filter_config_t audio_filter_config_bandstop(float fc1, float fc2, int order, float fs) {
    filter_config_t config = {0};
    config.type = FILTER_BANDSTOP;
    config.implementation = FILTER_IIR_BUTTERWORTH;
    config.fc1 = fc1;
    config.fc2 = fc2;
    config.order = order;
    config.sample_rate = fs;
    return config;
}
