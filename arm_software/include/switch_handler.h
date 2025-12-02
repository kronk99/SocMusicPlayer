/**
 * @file switch_handler.h
 * @brief FPGA Switch Handler for real-time filter control
 *
 * Lee los switches de la FPGA DE1-SoC y mapea a filtros de audio.
 * Permite cambiar filtros en tiempo real durante la reproducción.
 *
 * Mapeo de switches (SW0-SW9):
 * SW[3:0] - Selección de filtro (16 combinaciones):
 *   0000 (0)  - Sin filtro (bypass)
 *   0001 (1)  - Lowpass 10 kHz
 *   0010 (2)  - Lowpass 5 kHz
 *   0011 (3)  - Lowpass 2 kHz
 *   0100 (4)  - Highpass 100 Hz
 *   0101 (5)  - Highpass 500 Hz
 *   0110 (6)  - Highpass 1 kHz
 *   0111 (7)  - Bandpass 300 Hz - 3 kHz (telefónico)
 *   1000 (8)  - Bandpass 1 kHz - 5 kHz
 *   1001 (9)  - Bandpass 80 Hz - 250 Hz (sub-bass)
 *   1010 (10) - Lowpass 1 kHz (reduce agudos)
 *   1011 (11) - Highpass 2 kHz (solo agudos)
 *   1100 (12) - Bandpass 200 Hz - 800 Hz (radio AM)
 *   1101-1111 - Reservado
 */

#ifndef SWITCH_HANDLER_H
#define SWITCH_HANDLER_H

#include <stdint.h>
#include "audio_filter.h"

/* ============================================================================
 * SWITCH DEFINITIONS
 * ============================================================================ */

#define NUM_SWITCHES 10
#define FILTER_SELECT_MASK 0x0F  /* SW[3:0] para selección de filtro */

/**
 * @brief Filter presets mapped to switch values
 */
typedef enum {
    FILTER_PRESET_NONE = 0,          /* SW=0: Sin filtro */
    FILTER_PRESET_LP_10K = 1,        /* SW=1: Lowpass 10 kHz */
    FILTER_PRESET_LP_5K = 2,         /* SW=2: Lowpass 5 kHz */
    FILTER_PRESET_LP_2K = 3,         /* SW=3: Lowpass 2 kHz */
    FILTER_PRESET_HP_100 = 4,        /* SW=4: Highpass 100 Hz */
    FILTER_PRESET_HP_500 = 5,        /* SW=5: Highpass 500 Hz */
    FILTER_PRESET_HP_1K = 6,         /* SW=6: Highpass 1 kHz */
    FILTER_PRESET_BP_PHONE = 7,      /* SW=7: Bandpass telefónico */
    FILTER_PRESET_BP_MID = 8,        /* SW=8: Bandpass medios */
    FILTER_PRESET_BP_BASS = 9,       /* SW=9: Bandpass sub-bass */
    FILTER_PRESET_LP_1K = 10,        /* SW=10: Lowpass 1 kHz */
    FILTER_PRESET_HP_2K = 11,        /* SW=11: Highpass 2 kHz */
    FILTER_PRESET_BP_AM = 12,        /* SW=12: Bandpass AM radio */
    FILTER_PRESET_MAX = 13
} filter_preset_t;

/**
 * @brief Switch handler context
 */
typedef struct {
    volatile uint32_t *switches_addr;  /* Memory-mapped switches address */
    uint32_t last_value;               /* Previous switch value */
    filter_preset_t current_preset;    /* Current filter preset */
    int initialized;
} switch_handler_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize switch handler
 *
 * @param handler Pointer to switch_handler_t structure
 * @param switches_physical_addr Physical address of switches (0xFF200040)
 * @return 0 on success, -1 on error
 */
int switch_handler_init(switch_handler_t *handler, uint32_t switches_physical_addr);

/**
 * @brief Close switch handler
 *
 * @param handler Pointer to switch_handler_t structure
 */
void switch_handler_close(switch_handler_t *handler);

/**
 * @brief Read current switch values
 *
 * @param handler Pointer to switch_handler_t structure
 * @return Current switch value (10-bit)
 */
uint32_t switch_handler_read(switch_handler_t *handler);

/**
 * @brief Check if filter preset has changed
 *
 * @param handler Pointer to switch_handler_t structure
 * @return New preset if changed, -1 if no change
 */
int switch_handler_check_filter_change(switch_handler_t *handler);

/**
 * @brief Get filter configuration for preset
 *
 * @param preset Filter preset enum
 * @param fs Sample rate (Hz)
 * @return filter_config_t structure (type=FILTER_NONE if invalid preset)
 */
filter_config_t switch_handler_get_filter_config(filter_preset_t preset, float fs);

/**
 * @brief Get preset name string
 *
 * @param preset Filter preset enum
 * @return String name of preset
 */
const char* switch_handler_get_preset_name(filter_preset_t preset);

/**
 * @brief Print switch status and current filter
 *
 * @param handler Pointer to switch_handler_t structure
 */
void switch_handler_print_status(switch_handler_t *handler);

#endif /* SWITCH_HANDLER_H */
