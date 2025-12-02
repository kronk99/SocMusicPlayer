/**
 * @file display_driver.h
 * @brief 7-segment display driver
 *
 * Módulo para controlar los displays de 7 segmentos de la DE-SoC1.
 * Muestra el tiempo de reproducción en formato MM:SS
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>

/* ============================================================================
 * DISPLAY DEFINITIONS
 * ============================================================================ */

/**
 * @brief Number of 7-segment displays (DE-SoC1 has 6)
 */
#define NUM_DISPLAYS 6

/**
 * @brief 7-segment encoding table for digits 0-9
 *
 * Bit mapping (active low typical):
 *   Bit 0: Segment A (top)
 *   Bit 1: Segment B (top right)
 *   Bit 2: Segment C (bottom right)
 *   Bit 3: Segment D (bottom)
 *   Bit 4: Segment E (bottom left)
 *   Bit 5: Segment F (top left)
 *   Bit 6: Segment G (middle)
 *   Bit 7: Decimal point (if applicable)
 */

/**
 * @brief Display driver context
 */
typedef struct {
    volatile uint32_t *display_base;    /* Memory-mapped display register */
    uint32_t current_value;             /* Current displayed value */
    uint8_t digit_buffer[NUM_DISPLAYS]; /* Individual digit values */
} display_driver_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize display driver
 *
 * @param driver Pointer to display_driver_t structure
 * @param display_physical_addr Physical address of display PIO
 * @return 0 on success, -1 on error
 */
int display_driver_init(display_driver_t *driver, uint32_t display_physical_addr);

/**
 * @brief Shutdown display driver
 *
 * Clears display and unmaps memory.
 *
 * @param driver Pointer to display_driver_t structure
 */
void display_driver_shutdown(display_driver_t *driver);

/**
 * @brief Display time in MM:SS format
 *
 * Shows minutes and seconds on the 7-segment displays.
 * Format: MM.SS (using 4 displays, others can show track number)
 *
 * @param driver Pointer to display_driver_t structure
 * @param seconds Total seconds to display
 */
void display_driver_show_time(display_driver_t *driver, uint32_t seconds);

/**
 * @brief Display a raw hexadecimal value
 *
 * @param driver Pointer to display_driver_t structure
 * @param value Value to display (up to 6 hex digits)
 */
void display_driver_show_hex(display_driver_t *driver, uint32_t value);

/**
 * @brief Display a decimal value
 *
 * @param driver Pointer to display_driver_t structure
 * @param value Value to display (up to 999999)
 */
void display_driver_show_decimal(display_driver_t *driver, uint32_t value);

/**
 * @brief Set individual digit
 *
 * @param driver Pointer to display_driver_t structure
 * @param digit_index Display index (0-5, right to left typically)
 * @param value Digit value (0-9, or 0xFF for blank)
 */
void display_driver_set_digit(display_driver_t *driver, uint8_t digit_index, uint8_t value);

/**
 * @brief Clear all displays
 *
 * @param driver Pointer to display_driver_t structure
 */
void display_driver_clear(display_driver_t *driver);

/**
 * @brief Update display with current buffer contents
 *
 * Writes the digit_buffer to hardware.
 *
 * @param driver Pointer to display_driver_t structure
 */
void display_driver_update(display_driver_t *driver);

/**
 * @brief Test display (all segments on)
 *
 * @param driver Pointer to display_driver_t structure
 */
void display_driver_test(display_driver_t *driver);

/**
 * @brief Convert digit to 7-segment encoding
 *
 * @param digit Digit value (0-9)
 * @return 7-segment bit pattern
 */
uint8_t display_driver_digit_to_segments(uint8_t digit);

#endif /* DISPLAY_DRIVER_H */
