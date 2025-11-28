/**
 * @file config.h
 * @brief System configuration and hardware definitions
 *
 * Config for Soc Audio Player in Altera DE-SoC1
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * =========================================
 * HARDWARE MEMORY MAP
 * =========================================
 * TODO: This directions should be updated according to the design in Platform Designer.
 * The following values are just placeholders.
 */

/* HPS-FPGA Bridge Base Addresses */
#define HPS_TO_FPGA_BASE    0xC0000000 /* Main bridge */

/* FPGA Peripherals Offsets (adjust acoording to PLatform Designer) */
#define FIFO_BASE_OFFSET    0x10000000 /* Audio FIFO offset */
#define FIFO_SIZE           8192       /* 8KB FIFO */

/* Display Update Rate */
#define DISPLAY_UPDATE_MS   100


/* Playlist configuration */
#define MAX_PLAYLIST_SIZE   64
#define MUSIC_DIRECTORY     "/home/root/sounds"


/* Some macros */
#define INFO_PRINT(fmt, ...) \
    printf("[INFO] " fmt "\n", ##__VA_ARGS__)

#define DEBUG_PRINT(fmt, ...) \
    printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)



#endif /* CONFIG_H */

