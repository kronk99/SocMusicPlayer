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
// TODO: Rest of the includes


/* =======================
 * GLOBAL CONTEXT
 * ======================= */

typedef struct {
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

    /* TODO: Rest of them -> 2., 3., 4., etc*/


    INFO_PRINT("================================");
    INFO_PRINT("System initialized successfully!");
    INFO_PRINT("================================");

    return 0;
}

static void shutdown_system(app_context_t *app) {
    INFO_PRINT("Shutting down system...");

    (void)app;
    //TODO: Shutdown everything and remove previous line
   


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

