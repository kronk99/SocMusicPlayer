/**
 * @file main.c
 * @brief Main application for SoC Audio Player (ARM HPS side)
 *
 * Este es el orquestador principal del reproductor de audio.
 * Integra todos los módulos: WAV reader, FIFO writer, audio controller,
 * button handler, display driver, y web interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "config.h"
#include "wav_reader.h"
#include "shared_mem_writer.h"
#include "audio_controller.h"
#include "button_handler.h"
#include "display_driver.h"
#include "web_interface.h"
#include "switch_handler.h"
#include "audio_filter.h"

/* ============================================================================
 * GLOBAL CONTEXT
 * ============================================================================ */

typedef struct {
    shared_mem_context_t shmem;
    audio_controller_t audio;
    button_handler_t buttons;
    display_driver_t display;
    web_interface_t web;
    switch_handler_t switches;  /* FPGA switches for filter control */
    int running;
} app_context_t;

static app_context_t g_app;

/* ============================================================================
 * BUTTON CALLBACKS
 * ============================================================================ */

static void button_play_pause_callback(button_id_t button, button_event_t event, void *user_data) {
    (void)button;
    (void)event;

    app_context_t *app = (app_context_t *)user_data;

    playback_state_t state = audio_controller_get_state(&app->audio);

    if (state == STATE_STOPPED || state == STATE_PAUSED) {
        INFO_PRINT("Button: PLAY");
        audio_controller_play(&app->audio);
    } else if (state == STATE_PLAYING) {
        INFO_PRINT("Button: PAUSE");
        audio_controller_pause(&app->audio);
    }
}

static void button_next_callback(button_id_t button, button_event_t event, void *user_data) {
    (void)button;
    (void)event;

    app_context_t *app = (app_context_t *)user_data;
    INFO_PRINT("Button: NEXT");
    audio_controller_next(&app->audio);
}

static void button_prev_callback(button_id_t button, button_event_t event, void *user_data) {
    (void)button;
    (void)event;

    app_context_t *app = (app_context_t *)user_data;
    INFO_PRINT("Button: PREV");
    audio_controller_prev(&app->audio);
}

static void button_stop_callback(button_id_t button, button_event_t event, void *user_data) {
    (void)button;
    (void)event;

    app_context_t *app = (app_context_t *)user_data;
    INFO_PRINT("Button: STOP");
    audio_controller_stop(&app->audio);
}

/* ============================================================================
 * SIGNAL HANDLER
 * ============================================================================ */

static void signal_handler(int signum) {
    (void)signum;
    INFO_PRINT("Signal received, shutting down...");
    g_app.running = 0;
}

/* ============================================================================
 * INITIALIZATION & CLEANUP
 * ============================================================================ */

static int initialize_system(app_context_t *app) {
    memset(app, 0, sizeof(app_context_t));

    INFO_PRINT("========================================");
    INFO_PRINT("SoC Audio Player - ARM HPS");
    INFO_PRINT("========================================");

    /* 1. Initialize Shared Memory (ARM-NIOS communication via SDRAM) */
    INFO_PRINT("Initializing shared memory...");
    /* Using SDRAM @ 0xC0000000 + 1MB offset (SHARED_MEM_ARM_BASE) */
    if (shared_mem_init(&app->shmem, SHARED_MEM_ARM_BASE) != 0) {
        ERROR_PRINT("Failed to initialize shared memory");
        return -1;
    }

    /* 2. Initialize audio controller */
    INFO_PRINT("Initializing audio controller...");
    if (audio_controller_init(&app->audio, &app->shmem) != 0) {
        ERROR_PRINT("Failed to initialize audio controller");
        shared_mem_close(&app->shmem);
        return -1;
    }

    /* 3. Load playlist */
    INFO_PRINT("Loading playlist from %s...", MUSIC_DIRECTORY);
    int num_songs = audio_controller_load_playlist(&app->audio, MUSIC_DIRECTORY);
    if (num_songs <= 0) {
        ERROR_PRINT("No songs found in playlist");
        /* Continue anyway, but warn user */
    } else {
        INFO_PRINT("Loaded %d songs", num_songs);
    }

    /* 4. Initialize button handler */
    INFO_PRINT("Initializing buttons...");
    uint32_t button_addr = HPS_TO_FPGA_LW_BASE + BUTTONS_BASE_OFFSET;
    if (button_handler_init(&app->buttons, button_addr, 0) != 0) {
        ERROR_PRINT("Failed to initialize button handler");
        /* Non-fatal, continue */
    } else {
        /* Register button callbacks */
        button_handler_register_callback(&app->buttons, BUTTON_PLAY_PAUSE,
                                          button_play_pause_callback, app);
        button_handler_register_callback(&app->buttons, BUTTON_NEXT,
                                          button_next_callback, app);
        button_handler_register_callback(&app->buttons, BUTTON_PREV,
                                          button_prev_callback, app);
        button_handler_register_callback(&app->buttons, BUTTON_STOP,
                                          button_stop_callback, app);
    }

    /* 5. Initialize 7-segment display */
    INFO_PRINT("Initializing display...");
    uint32_t display_addr = HPS_TO_FPGA_LW_BASE + DISPLAY_BASE_OFFSET;
    if (display_driver_init(&app->display, display_addr) != 0) {
        ERROR_PRINT("Failed to initialize display driver");
        /* Non-fatal, continue */
    }

    /* 5b. Initialize switch handler for real-time filter control */
    INFO_PRINT("Initializing switches for filter control...");
    uint32_t switches_addr = HPS_TO_FPGA_LW_BASE + SWITCHES_BASE_OFFSET;
    if (switch_handler_init(&app->switches, switches_addr) != 0) {
        ERROR_PRINT("Failed to initialize switch handler");
        /* Non-fatal, continue */
    } else {
        switch_handler_print_status(&app->switches);
    }

    /* 6. Initialize web interface */
    INFO_PRINT("Initializing web interface on port %d...", WEB_SERVER_PORT);
    if (web_interface_init(&app->web, &app->audio, WEB_SERVER_PORT) != 0) {
        ERROR_PRINT("Failed to initialize web interface");
        /* Non-fatal, continue */
    }

    /* 7. Anti-aliasing filter disabled by default - controlled by switches */
    INFO_PRINT("Audio filters disabled by default");
    INFO_PRINT("  Use SW[3:0] to enable filters:");

    INFO_PRINT("========================================");
    INFO_PRINT("System initialized successfully!");
    INFO_PRINT("Web interface: http://<board-ip>:%d", WEB_SERVER_PORT);
    INFO_PRINT("Use SW[3:0] switches to change audio filters in real-time");
    INFO_PRINT("  SW=0: Disable anti-aliasing (bypass)");
    INFO_PRINT("  SW=1: Anti-aliasing 10 kHz");
    INFO_PRINT("  SW=2-12: Various filter presets");
    INFO_PRINT("========================================");

    return 0;
}

static void shutdown_system(app_context_t *app) {
    INFO_PRINT("Shutting down system...");

    /* Shutdown in reverse order */
    web_interface_shutdown(&app->web);
    switch_handler_close(&app->switches);
    display_driver_shutdown(&app->display);
    button_handler_shutdown(&app->buttons);
    audio_controller_shutdown(&app->audio);
    shared_mem_close(&app->shmem);

    INFO_PRINT("Shutdown complete");
}

/* ============================================================================
 * MAIN LOOP
 * ============================================================================ */

static void run_main_loop(app_context_t *app) {
    INFO_PRINT("Entering main loop...");

    app->running = 1;

    while (app->running) {
        /* Check switches for filter changes (real-time filter control) */
        int filter_preset = switch_handler_check_filter_change(&app->switches);
        if (filter_preset >= 0) {
            /* Switch changed, update filter */
            filter_config_t config = switch_handler_get_filter_config(
                (filter_preset_t)filter_preset, AUDIO_SAMPLE_RATE);
            INFO_PRINT("Applying filter: %s",
                       switch_handler_get_preset_name((filter_preset_t)filter_preset));
            audio_controller_set_filter(&app->audio, &config);
        }

        /* Update display with current playback time */
        uint32_t current_sec, total_sec;
        audio_controller_get_time(&app->audio, &current_sec, &total_sec);
        display_driver_show_time(&app->display, current_sec);

        /* Update web interface status */
        web_interface_update_status(&app->web);

        /* Print status periodically (every 5 seconds) */
        static uint32_t last_status_print = 0;
        if (current_sec > last_status_print + 5) {
            playback_state_t state = audio_controller_get_state(&app->audio);
            const playlist_entry_t *track = audio_controller_get_current_track(&app->audio);

            if (state == STATE_PLAYING && track) {
                INFO_PRINT("Playing: %s - %02u:%02u / %02u:%02u",
                           track->title,
                           current_sec / 60, current_sec % 60,
                           total_sec / 60, total_sec % 60);
            }

            last_status_print = current_sec;
        }

        /* Check shared memory status (debug) */
        if (DEBUG_FIFO_STATUS) {
            uint32_t available = shared_mem_get_available(&app->shmem);
            DEBUG_PRINT("Shared buffer: %u samples available", available);
        }

        /* Sleep to avoid busy-waiting */
        usleep(DISPLAY_UPDATE_MS * 1000);
    }

    INFO_PRINT("Exiting main loop");
}

/* ============================================================================
 * MAIN ENTRY POINT
 * ============================================================================ */

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║   SoC Audio Player - ARM Cortex-A9    ║\n");
    printf("║   Plataforma: Altera DE-SoC1           ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");

    /* Check if running as root (required for /dev/mem access) */
    if (geteuid() != 0) {
        ERROR_PRINT("This application must be run as root (for /dev/mem access)");
        ERROR_PRINT("Try: sudo %s", argv[0]);
        return EXIT_FAILURE;
    }

    /* Initialize system */
    if (initialize_system(&g_app) != 0) {
        ERROR_PRINT("System initialization failed");
        return EXIT_FAILURE;
    }

    /* Run main loop */
    run_main_loop(&g_app);

    /* Cleanup */
    shutdown_system(&g_app);

    printf("\nGoodbye!\n");
    return EXIT_SUCCESS;
}
