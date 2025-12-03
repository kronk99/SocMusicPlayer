/**
 * @file switch_handler.c
 * @brief FPGA Switch Handler Implementation
 */

#include "switch_handler.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

/* ============================================================================
 * FILTER PRESET DEFINITIONS
 * ============================================================================ */

/**
 * @brief Filter preset name strings
 */
static const char* preset_names[] = {
    "None (Bypass)",
    "MUTE (Silence)",
    "Volume HIGH (x3)",
    "Distortion (Extreme)"
};

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

int switch_handler_init(switch_handler_t *handler, uint32_t switches_physical_addr) {
    if (!handler) {
        ERROR_PRINT("Invalid handler");
        return -1;
    }

    memset(handler, 0, sizeof(switch_handler_t));

    /* Open /dev/mem */
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem. Are you running as root?");
        return -1;
    }

    /* Align to page boundary */
    uint32_t page_aligned = switches_physical_addr & ~PAGE_MASK;
    uint32_t offset_in_page = switches_physical_addr & PAGE_MASK;

    /* Memory map */
    void *mapped = mmap(NULL, PAGE_SIZE,
                        PROT_READ,
                        MAP_SHARED,
                        fd, page_aligned);
    close(fd);

    if (mapped == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap switches at 0x%08X", switches_physical_addr);
        return -1;
    }

    /* Setup pointer */
    handler->switches_addr = (volatile uint32_t *)((uint8_t *)mapped + offset_in_page);
    handler->last_value = *handler->switches_addr & 0x3FF;  /* 10-bit */
    handler->current_preset = (filter_preset_t)(handler->last_value & FILTER_SELECT_MASK);
    handler->initialized = 1;

    INFO_PRINT("Switch handler initialized");
    INFO_PRINT("  Address: 0x%08X", switches_physical_addr);
    INFO_PRINT("  Initial value: 0x%03X", handler->last_value);
    INFO_PRINT("  Initial preset: %s", switch_handler_get_preset_name(handler->current_preset));

    return 0;
}

void switch_handler_close(switch_handler_t *handler) {
    if (handler && handler->initialized) {
        /* Note: We don't store the mapped base, so can't munmap cleanly
         * In practice, OS will clean up on process exit */
        handler->initialized = 0;
        INFO_PRINT("Switch handler closed");
    }
}

uint32_t switch_handler_read(switch_handler_t *handler) {
    if (!handler || !handler->initialized) {
        return 0;
    }

    return *handler->switches_addr & 0x3FF;  /* 10-bit mask */
}

int switch_handler_check_filter_change(switch_handler_t *handler) {
    if (!handler || !handler->initialized) {
        return -1;
    }

    uint32_t current_value = switch_handler_read(handler);
    filter_preset_t current_preset = (filter_preset_t)(current_value & FILTER_SELECT_MASK);

    if (current_preset != handler->current_preset) {
        /* Filter changed */
        INFO_PRINT("Switch changed: 0x%03X -> 0x%03X", handler->last_value, current_value);
        INFO_PRINT("Filter preset: %s -> %s",
                   switch_handler_get_preset_name(handler->current_preset),
                   switch_handler_get_preset_name(current_preset));

        handler->last_value = current_value;
        handler->current_preset = current_preset;

        return (int)current_preset;
    }

    return -1;  /* No change */
}

filter_config_t switch_handler_get_filter_config(filter_preset_t preset, float fs) {
    filter_config_t config = {0};
    config.sample_rate = fs;

    switch (preset) {
        case FILTER_PRESET_NONE:
            config.type = FILTER_NONE;
            break;

        case FILTER_PRESET_LP_10K:
            /* SW=1: MUTE */
            config.type = FILTER_MUTE;
            break;

        case FILTER_PRESET_LP_5K:
            /* SW=2: Volume High */
            config.type = FILTER_VOLUME_HIGH;
            break;

        case FILTER_PRESET_LP_2K:
            /* SW=3: Distortion Extreme */
            config.type = FILTER_DISTORTION;
            break;

        default:
            /* Cualquier otro switch = bypass */
            config.type = FILTER_NONE;
            break;
    }

    return config;
}

const char* switch_handler_get_preset_name(filter_preset_t preset) {
    if (preset < 0 || preset > 3) {
        return "Bypass";  /* Cualquier switch > 3 = bypass */
    }
    return preset_names[preset];
}

void switch_handler_print_status(switch_handler_t *handler) {
    if (!handler || !handler->initialized) {
        printf("Switch handler not initialized\n");
        return;
    }

    uint32_t value = switch_handler_read(handler);

    printf("========================================\n");
    printf("FPGA Switches Status\n");
    printf("========================================\n");
    printf("Switch Value:    0x%03X (0b", value);

    /* Print binary */
    for (int i = 9; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i == 4) printf(" ");  /* Separate high/low nibble */
    }
    printf(")\n");

    /* Individual switches */
    printf("Switches:        ");
    for (int i = 9; i >= 0; i--) {
        printf("SW%d:%d ", i, (value >> i) & 1);
    }
    printf("\n");

    printf("Filter Select:   SW[3:0] = %d\n", handler->current_preset);
    printf("Current Filter:  %s\n", switch_handler_get_preset_name(handler->current_preset));
    printf("========================================\n");
}
