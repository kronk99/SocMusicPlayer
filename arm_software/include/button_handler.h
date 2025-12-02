/**
 * @file button_handler.h
 * @brief Button interrupt handler
 *
 * Módulo para manejar interrupciones de botones en la DE-SoC1.
 * Implementa debouncing y callbacks para eventos de botones.
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdint.h>
#include <pthread.h>

/* ============================================================================
 * BUTTON DEFINITIONS
 * ============================================================================ */

/**
 * @brief Button IDs
 */
typedef enum {
    BUTTON_PLAY_PAUSE = 0,  /* KEY0 */
    BUTTON_NEXT = 1,        /* KEY1 */
    BUTTON_PREV = 2,        /* KEY2 */
    BUTTON_STOP = 3,        /* KEY3 */
    BUTTON_COUNT = 4
} button_id_t;

/**
 * @brief Button event types
 */
typedef enum {
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_t;

/**
 * @brief Button callback function type
 *
 * @param button Button that triggered the event
 * @param event Type of event (pressed/released)
 * @param user_data User-provided data pointer
 */
typedef void (*button_callback_t)(button_id_t button, button_event_t event, void *user_data);

/**
 * @brief Button handler context
 */
typedef struct {
    volatile uint32_t *button_base;     /* Memory-mapped button register */
    volatile uint32_t *irq_mask_base;   /* Interrupt mask register */
    volatile uint32_t *edge_cap_base;   /* Edge capture register */

    /* Callbacks */
    button_callback_t callbacks[BUTTON_COUNT];
    void *callback_data[BUTTON_COUNT];

    /* Interrupt handling */
    pthread_t irq_thread;
    int irq_fd;                         /* File descriptor for UIO device */
    int thread_running;

    /* Debouncing */
    uint32_t last_button_state;
    uint64_t last_press_time[BUTTON_COUNT];
    uint32_t debounce_ms;

    /* Statistics */
    uint32_t button_press_count[BUTTON_COUNT];
} button_handler_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize button handler
 *
 * Sets up memory-mapped I/O and interrupt handling.
 *
 * @param handler Pointer to button_handler_t structure
 * @param button_physical_addr Physical address of button PIO
 * @param irq_num IRQ number (for UIO device)
 * @return 0 on success, -1 on error
 */
int button_handler_init(button_handler_t *handler, uint32_t button_physical_addr, int irq_num);

/**
 * @brief Shutdown button handler
 *
 * Stops interrupt thread and cleans up resources.
 *
 * @param handler Pointer to button_handler_t structure
 */
void button_handler_shutdown(button_handler_t *handler);

/**
 * @brief Register callback for button event
 *
 * @param handler Pointer to button_handler_t structure
 * @param button Button ID to register callback for
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return 0 on success, -1 on error
 */
int button_handler_register_callback(button_handler_t *handler,
                                      button_id_t button,
                                      button_callback_t callback,
                                      void *user_data);

/**
 * @brief Read current button state (polling mode)
 *
 * @param handler Pointer to button_handler_t structure
 * @return Button state bitmask (bit 0 = KEY0, bit 1 = KEY1, etc.)
 */
uint32_t button_handler_read_state(button_handler_t *handler);

/**
 * @brief Set debounce time
 *
 * @param handler Pointer to button_handler_t structure
 * @param ms Debounce time in milliseconds
 */
void button_handler_set_debounce(button_handler_t *handler, uint32_t ms);

/**
 * @brief Enable button interrupts
 *
 * @param handler Pointer to button_handler_t structure
 * @param button_mask Bitmask of buttons to enable (bit 0 = KEY0, etc.)
 */
void button_handler_enable_interrupts(button_handler_t *handler, uint32_t button_mask);

/**
 * @brief Disable button interrupts
 *
 * @param handler Pointer to button_handler_t structure
 * @param button_mask Bitmask of buttons to disable
 */
void button_handler_disable_interrupts(button_handler_t *handler, uint32_t button_mask);

/**
 * @brief Get button press statistics
 *
 * @param handler Pointer to button_handler_t structure
 * @param button Button ID
 * @return Number of times button has been pressed
 */
uint32_t button_handler_get_press_count(button_handler_t *handler, button_id_t button);

/**
 * @brief Print button statistics (debug)
 *
 * @param handler Pointer to button_handler_t structure
 */
void button_handler_print_stats(button_handler_t *handler);

#endif /* BUTTON_HANDLER_H */
