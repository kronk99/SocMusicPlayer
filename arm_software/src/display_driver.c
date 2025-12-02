/**
 * @file display_driver.c
 * @brief 7-segment display driver implementation
 */

#include "display_driver.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================ */

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

/**
 * @brief 7-segment encoding for digits 0-9 (active low, common anode)
 * Segments: GFEDCBA (bit 6 to bit 0)
 */
static const uint8_t SEGMENT_MAP[10] = {
    0x3F,  /* 0: 0111111 */
    0x06,  /* 1: 0000110 */
    0x5B,  /* 2: 1011011 */
    0x4F,  /* 3: 1001111 */
    0x66,  /* 4: 1100110 */
    0x6D,  /* 5: 1101101 */
    0x7D,  /* 6: 1111101 */
    0x07,  /* 7: 0000111 */
    0x7F,  /* 8: 1111111 */
    0x6F   /* 9: 1101111 */
};

#define SEGMENT_BLANK 0x00

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

int display_driver_init(display_driver_t *driver, uint32_t display_physical_addr) {
    if (!driver) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(driver, 0, sizeof(display_driver_t));

    /* Open /dev/mem */
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem");
        return -1;
    }

    /* Align and map */
    uint32_t page_aligned = display_physical_addr & ~PAGE_MASK;
    uint32_t offset = display_physical_addr & PAGE_MASK;

    void *mapped = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_aligned);
    close(fd);

    if (mapped == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap display registers");
        return -1;
    }

    driver->display_base = (volatile uint32_t *)((uint8_t *)mapped + offset);

    /* Clear display */
    display_driver_clear(driver);

    INFO_PRINT("Display driver initialized at 0x%08X", display_physical_addr);
    return 0;
}

void display_driver_shutdown(display_driver_t *driver) {
    if (!driver) return;

    /* Clear display */
    display_driver_clear(driver);

    if (driver->display_base) {
        munmap((void *)driver->display_base, PAGE_SIZE);
    }

    INFO_PRINT("Display driver shutdown");
}

void display_driver_show_time(display_driver_t *driver, uint32_t seconds) {
    if (!driver) return;

    uint32_t minutes = seconds / 60;
    uint32_t secs = seconds % 60;

    /* Limit to 99:59 */
    if (minutes > 99) minutes = 99;

    /* Extract digits: MM.SS */
    uint8_t min_tens = (minutes / 10) % 10;
    uint8_t min_ones = minutes % 10;
    uint8_t sec_tens = (secs / 10) % 10;
    uint8_t sec_ones = secs % 10;

    /* Set digits (assuming displays 0-5, right to left) */
    /* Layout: HEX5 HEX4 HEX3 HEX2 HEX1 HEX0 */
    /*         -    -    M    M    S    S    */
    driver->digit_buffer[0] = sec_ones;   /* Rightmost */
    driver->digit_buffer[1] = sec_tens;
    driver->digit_buffer[2] = min_ones;
    driver->digit_buffer[3] = min_tens;
    driver->digit_buffer[4] = 0xFF;       /* Blank */
    driver->digit_buffer[5] = 0xFF;       /* Blank */

    display_driver_update(driver);
}

void display_driver_show_hex(display_driver_t *driver, uint32_t value) {
    if (!driver) return;

    /* Extract 6 hex digits */
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        uint8_t nibble = (value >> (i * 4)) & 0x0F;
        driver->digit_buffer[i] = nibble;
    }

    display_driver_update(driver);
}

void display_driver_show_decimal(display_driver_t *driver, uint32_t value) {
    if (!driver) return;

    /* Extract decimal digits */
    for (int i = 0; i < NUM_DISPLAYS; i++) {
        driver->digit_buffer[i] = value % 10;
        value /= 10;
    }

    display_driver_update(driver);
}

void display_driver_set_digit(display_driver_t *driver, uint8_t digit_index, uint8_t value) {
    if (!driver || digit_index >= NUM_DISPLAYS) return;

    driver->digit_buffer[digit_index] = value;
}

void display_driver_clear(display_driver_t *driver) {
    if (!driver || !driver->display_base) return;

    /* Write all zeros */
    *driver->display_base = 0x00000000;
    driver->current_value = 0;

    memset(driver->digit_buffer, 0xFF, sizeof(driver->digit_buffer));
}

void display_driver_update(display_driver_t *driver) {
    if (!driver || !driver->display_base) return;

    /* Encode all digits into 32-bit value */
    /* Each digit uses 8 bits (7 segments + DP) */
    /* Layout depends on hardware - this is a typical configuration */
    uint32_t display_value = 0;

    for (int i = 0; i < NUM_DISPLAYS && i < 4; i++) {
        uint8_t segments;
        if (driver->digit_buffer[i] == 0xFF) {
            segments = SEGMENT_BLANK;
        } else if (driver->digit_buffer[i] < 10) {
            segments = SEGMENT_MAP[driver->digit_buffer[i]];
        } else {
            /* Hex digits A-F (if needed) */
            segments = SEGMENT_BLANK;
        }
        display_value |= ((uint32_t)segments << (i * 8));
    }

    /* Write to hardware */
    *driver->display_base = display_value;
    driver->current_value = display_value;

    DEBUG_PRINT("Display updated: 0x%08X", display_value);
}

void display_driver_test(display_driver_t *driver) {
    if (!driver || !driver->display_base) return;

    INFO_PRINT("Display test: all segments ON");

    /* All segments on */
    *driver->display_base = 0xFFFFFFFF;
    sleep(1);

    /* All segments off */
    display_driver_clear(driver);
}

uint8_t display_driver_digit_to_segments(uint8_t digit) {
    if (digit < 10) {
        return SEGMENT_MAP[digit];
    }
    return SEGMENT_BLANK;
}
