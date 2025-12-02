/**
 * @file button_handler.c
 * @brief Button interrupt handler implementation
 */

#include "button_handler.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================ */

#define DEFAULT_DEBOUNCE_MS 50
#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get current time in milliseconds
 */
static uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

/**
 * @brief Check if button press is valid (debouncing)
 */
static int is_valid_press(button_handler_t *handler, button_id_t button) {
    uint64_t now = get_time_ms();
    uint64_t elapsed = now - handler->last_press_time[button];

    if (elapsed < handler->debounce_ms) {
        return 0; /* Too soon, ignore */
    }

    handler->last_press_time[button] = now;
    return 1;
}

/**
 * @brief Interrupt thread function (polling mode fallback)
 */
static void* irq_thread_func(void *arg) {
    button_handler_t *handler = (button_handler_t *)arg;

    INFO_PRINT("Button handler thread started (polling mode)");

    while (handler->thread_running) {
        /* Read current button state */
        uint32_t current_state = *handler->button_base;

        /* Detect button presses (assuming active low) */
        uint32_t pressed = (~current_state) & 0x0F; /* Mask for 4 buttons */

        for (int i = 0; i < BUTTON_COUNT; i++) {
            if (pressed & (1 << i)) {
                /* Button is pressed */
                if (!(handler->last_button_state & (1 << i))) {
                    /* New press detected */
                    if (is_valid_press(handler, i)) {
                        handler->button_press_count[i]++;

                        /* Call callback if registered */
                        if (handler->callbacks[i]) {
                            handler->callbacks[i](i, BUTTON_EVENT_PRESSED, handler->callback_data[i]);
                        }
                    }
                }
            }
        }

        handler->last_button_state = pressed;

        usleep(10000); /* Poll every 10ms */
    }

    INFO_PRINT("Button handler thread stopped");
    return NULL;
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

int button_handler_init(button_handler_t *handler, uint32_t button_physical_addr, int irq_num) {
    if (!handler) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(handler, 0, sizeof(button_handler_t));

    /* Open /dev/mem */
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        ERROR_PRINT("Failed to open /dev/mem");
        return -1;
    }

    /* Align and map */
    uint32_t page_aligned = button_physical_addr & ~PAGE_MASK;
    uint32_t offset = button_physical_addr & PAGE_MASK;

    void *mapped = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_aligned);
    close(fd);

    if (mapped == MAP_FAILED) {
        ERROR_PRINT("Failed to mmap button registers");
        return -1;
    }

    handler->button_base = (volatile uint32_t *)((uint8_t *)mapped + offset);
    handler->debounce_ms = DEFAULT_DEBOUNCE_MS;

    /* Start polling thread */
    handler->thread_running = 1;
    if (pthread_create(&handler->irq_thread, NULL, irq_thread_func, handler) != 0) {
        ERROR_PRINT("Failed to create button thread");
        munmap(mapped, PAGE_SIZE);
        return -1;
    }

    INFO_PRINT("Button handler initialized at 0x%08X", button_physical_addr);
    return 0;
}

void button_handler_shutdown(button_handler_t *handler) {
    if (!handler) return;

    INFO_PRINT("Shutting down button handler");

    handler->thread_running = 0;
    pthread_join(handler->irq_thread, NULL);

    if (handler->button_base) {
        munmap((void *)handler->button_base, PAGE_SIZE);
    }

    INFO_PRINT("Button handler shutdown complete");
}

int button_handler_register_callback(button_handler_t *handler,
                                      button_id_t button,
                                      button_callback_t callback,
                                      void *user_data) {
    if (!handler || button >= BUTTON_COUNT) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    handler->callbacks[button] = callback;
    handler->callback_data[button] = user_data;

    DEBUG_PRINT("Registered callback for button %d", button);
    return 0;
}

uint32_t button_handler_read_state(button_handler_t *handler) {
    if (!handler || !handler->button_base) {
        return 0;
    }
    return *handler->button_base;
}

void button_handler_set_debounce(button_handler_t *handler, uint32_t ms) {
    if (handler) {
        handler->debounce_ms = ms;
        DEBUG_PRINT("Debounce time set to %u ms", ms);
    }
}

void button_handler_enable_interrupts(button_handler_t *handler, uint32_t button_mask) {
    /* TODO: Implement hardware interrupt enabling if supported */
    (void)handler;
    (void)button_mask;
}

void button_handler_disable_interrupts(button_handler_t *handler, uint32_t button_mask) {
    /* TODO: Implement hardware interrupt disabling if supported */
    (void)handler;
    (void)button_mask;
}

uint32_t button_handler_get_press_count(button_handler_t *handler, button_id_t button) {
    if (!handler || button >= BUTTON_COUNT) {
        return 0;
    }
    return handler->button_press_count[button];
}

void button_handler_print_stats(button_handler_t *handler) {
    if (!handler) return;

    printf("========================================\n");
    printf("Button Statistics\n");
    printf("========================================\n");
    for (int i = 0; i < BUTTON_COUNT; i++) {
        printf("Button %d: %u presses\n", i, handler->button_press_count[i]);
    }
    printf("========================================\n");
}
