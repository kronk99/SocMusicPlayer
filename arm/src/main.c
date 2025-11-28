/** 
 * @file main.c
 * @brief Main application for SoC Audio Player (ARM HPS side)
 * 
 * This is the main orquestrator for the music player.
 * TODO: Integrate all the modules:
 *       - WAV reader (TODO)
 *       - FIFO writer (Working on it)
 *       - Audio controller (TODO) 
 *       - Button handler (TODO)
 *       - Display driver (TODO)
 *       - Web interface (TODO)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "fifo_writer.h"
#include "wav_reader.h"
#include "audio_controller.h"
// TODO: Rest of the includes
// TODO: Get rid of the following include, its just for testing purposes


/* =======================
 * GLOBAL CONTEXT
 * ======================= */

typedef struct {
    audio_controller_t audio;
    fifo_context_t fifo;
} app_context_t; 

static app_context_t g_app;



/* =========================
 * INITIALIZATION & CLEANUP
 * ========================= */
static int initialize_system(app_context_t *app) {
    memset(app, 0, sizeof(app_context_t));

    INFO_PRINT("===============================");
    INFO_PRINT("SoC Audio Player - ARM HPS     ");
    INFO_PRINT("===============================");

    // TODO: Update addresses to match the Platform Designer
    
    /* 1. Initialize FIFO (HPS-FPGA communication) */
    INFO_PRINT("Initializing FIFO...");
    uint32_t fifo_addr = HPS_TO_FPGA_BASE + FIFO_BASE_OFFSET;

    if (fifo_init(&app->fifo, fifo_addr, FIFO_SIZE) != 0) {
        ERROR_PRINT("Failed to initialize FIFO");
        return -1;
    }

    /* 2. Initialize audio controller */
    INFO_PRINT("Initializing audio_controller...");
    if (audio_controller_init(&app->audio, &app->fifo) != 0) {
        ERROR_PRINT("Failed to initialize audio controller");
        fifo_close(&app->fifo);
        return -1;
    }

    /* 3. Load playlist */
    INFO_PRINT("Loading playlist from %s...", MUSIC_DIRECTORY);
    int num_songs = audio_controller_load_playlist(&app->audio, MUSIC_DIRECTORY);
    if (num_songs <= 0) { // TODO: This is causing a bug and it SHOULD be resolved. It prints it anyway
        ERROR_PRINT("No songs found in playlist");
        /* Continue anyway, but warned user */
    } else {
        INFO_PRINT("Loaded %d songs", num_songs);
    }

    /* TODO: Rest of them -> 3., 4., etc*/
    // TODO: Get rid of the following, its just for testing purposes
    //INFO_PRINT("Initializing wav reader...");

    //wav_file_t temp_wav;
    //wav_open(&temp_wav, "bass-wiggle-297877.wav");
    //wav_close(&temp_wav);
    // Until here, it is just for testing purposes


    INFO_PRINT("================================");
    INFO_PRINT("System initialized successfully!");
    INFO_PRINT("================================");

    // TODO: Get rid of the following, its just for testing purposes
    INFO_PRINT("Playing songs...");
    audio_controller_play(&app->audio);

    // Until here, it is just for testing purposes only

    return 0;
}

static void shutdown_system(app_context_t *app) {
    INFO_PRINT("Shutting down system...");

    //TODO: Shutdown everything else
    audio_controller_shutdown(&app->audio);
    fifo_close(&app->fifo);


    INFO_PRINT("Shutting complete");
}






/* =========================
 * MAIN ENTRY POINT
 * ========================= */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("╔═══════════════════════════════════════╗\n");
    printf("║   SoC Audio Player - ARM Cortex-A9    ║\n");
    printf("║   Plataforma: Altera DE-SoC1          ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    printf("\n");

    /* Initialize system */
    if (initialize_system(&g_app) != 0) {
        ERROR_PRINT("System initialization failed");
        return EXIT_FAILURE;
    }


    /* Cleanup */
    shutdown_system(&g_app);

    printf("\nGoodbye!\n");
    return EXIT_SUCCESS;
}

