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

    /* Simple effects (no complex implementation) */
    if (config->type == FILTER_NONE) {
        filter->enabled = 0;
        INFO_PRINT("Filter type NONE - bypass mode");
        return 0;
    }

    if (config->type == FILTER_MUTE) {
        filter->enabled = 1;
        INFO_PRINT("MUTE filter initialized (silence)");
        return 0;
    }

    if (config->type == FILTER_VOLUME_HIGH) {
        filter->enabled = 1;
        INFO_PRINT("Volume High filter initialized (x3)");
        return 0;
    }

    if (config->type == FILTER_DISTORTION) {
        filter->enabled = 1;
        INFO_PRINT("Distortion filter initialized (extreme clipping)");
        return 0;
    } 
    
    else {
        ERROR_PRINT("Unsupported filter implementation");
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

    /* Simple effects */
    if (filter->config.type == FILTER_MUTE) {
        filter->samples_processed++;
        return 0;
    }

    if (filter->config.type == FILTER_VOLUME_HIGH) {
        /* Volumen x3 con saturación */
        int32_t amplified = (int32_t)input * 8;
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        filter->samples_processed++;
        return (int16_t)amplified;
    }

    if (filter->config.type == FILTER_DISTORTION) {
        /* Distorsión EXTREMA: clipea a 1/4 del rango */
        const int16_t CLIP_THRESHOLD = 21000;  /* 32767 / 4 */
        int16_t clipped;

        if (input > CLIP_THRESHOLD) {
            clipped = CLIP_THRESHOLD;
        } else if (input < -CLIP_THRESHOLD) {
            clipped = -CLIP_THRESHOLD;
        } else {
            clipped = input;
        }

        filter->samples_processed++;
        return clipped;
    }

    /* Convert to float (-1.0 to 1.0 range) */
    float x = (float)input / 32768.0f;
    float y = x;

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

void audio_filter_enable(audio_filter_t *filter, int enable) {
    if (!filter) return;
    filter->enabled = enable;
}

void audio_filter_print_info(audio_filter_t *filter) {
    if (!filter) return;

    printf("========================================\n");
    printf("Audio Filter Information\n");
    printf("========================================\n");

    /* Parameters */
    printf("Sample Rate:     %.0f Hz\n", filter->config.sample_rate);

    printf("Status:          %s\n", filter->enabled ? "Enabled" : "Disabled");
    printf("Samples Proc:    %llu\n", (unsigned long long)filter->samples_processed);
    printf("========================================\n");
}
