/**
 * @file config.h
 * @brief System configuration and hardware definitions
 * @date 2025
 *
 * Configuración del sistema SoC Audio Player
 * Plataforma: Altera DE-SoC1 (Cyclone V)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* ============================================================================
 * HARDWARE MEMORY MAP
 * ============================================================================
 * DIRECCIONES PARA Computer_System_Fixed.qsys (diseño limpio)
 * Basado en: Table 5, página 24 del PDF DE1-SoC_Computer_NiosII.pdf
 * Verificado con: Computer_System_Fixed.qsys (19 componentes, sin VGA/Video/PS2)
 * Compatible con: Quartus 18.1
 */

/* HPS-FPGA Bridge Base Addresses */
#define HPS_TO_FPGA_LW_BASE     0xFF200000  /* Lightweight bridge (periféricos) */
#define HPS_TO_FPGA_BASE        0xC0000000  /* Main bridge (SDRAM access) */

/* FPGA Peripherals - Computer_System_Fixed */
#define LEDS_BASE               0xFF200000  /* Red LEDs (10-bit) */
#define HEX3_HEX0_BASE          0xFF200020  /* 7-segment displays 0-3 */
#define HEX5_HEX4_BASE          0xFF200030  /* 7-segment displays 4-5 */
#define SWITCHES_BASE           0xFF200040  /* Slider Switches (10-bit) */
#define BUTTONS_BASE            0xFF200050  /* Pushbutton KEYs (4-bit) */
#define JTAG_UART_BASE          0xFF201000  /* JTAG UART (debug) */
#define TIMER_BASE              0xFF202000  /* Interval Timer */
#define AV_CONFIG_BASE          0xFF203000  /* Audio/Video Config (I2C for WM8731) */
#define AUDIO_BASE              0xFF203040  /* Audio Subsystem (WM8731 codec) */

/* Shared Memory - ARM to NIOS via SDRAM (NO CACHE!) */
#define SHARED_MEM_ARM_BASE     0xE0100000  /* ARM: Uncached SDRAM + 1MB offset */
#define SHARED_MEM_NIOS_BASE    0x00100000  /* NIOS: SDRAM + 1MB offset */
#define SHARED_MEM_SIZE         (256 * 1024) /* 256KB buffer in SDRAM (65536 samples) */

/* Offsets relativos (para compatibilidad con código existente) */
#define DISPLAY_BASE_OFFSET     0x00000020  /* Offset desde LW_BASE */
#define SWITCHES_BASE_OFFSET    0x00000040  /* Offset desde LW_BASE */
#define BUTTONS_BASE_OFFSET     0x00000050  /* Offset desde LW_BASE */
#define LEDS_BASE_OFFSET        0x00000000  /* Offset desde LW_BASE */

/* ============================================================================
 * AUDIO CONFIGURATION
 * ============================================================================ */

/* WAV File Format */
#define AUDIO_SAMPLE_RATE       48000       /* Hz */
#define AUDIO_BITS_PER_SAMPLE   16          /* bits */
#define AUDIO_CHANNELS          1           /* Mono */
#define AUDIO_BYTES_PER_SAMPLE  2           /* 16-bit = 2 bytes */

/* Playback Buffer */
#define PLAYBACK_BUFFER_SIZE    4096        /* Samples */
#define FIFO_WRITE_CHUNK        256         /* Samples per write */

/* ============================================================================
 * PLAYLIST CONFIGURATION
 * ============================================================================ */

#define MAX_PLAYLIST_SIZE       64          /* Maximum songs */
#define MAX_FILENAME_LEN        256         /* Path length */
#define SD_MOUNT_POINT          "/mnt/sd"   /* SD card mount point */
#define MUSIC_DIRECTORY         "/home/root/sounds"

/* ============================================================================
 * USER INTERFACE CONFIGURATION
 * ============================================================================ */

/* Button IDs definidos en button_handler.h como enum */
/* Ver: button_handler.h para button_id_t enum */

/* Display Update Rate */
#define DISPLAY_UPDATE_MS       100         /* Update every 100ms */

/* Web Server */
#define WEB_SERVER_PORT         8080
#define WEB_ROOT_DIR            "./www"

/* ============================================================================
 * SYSTEM CONFIGURATION
 * ============================================================================ */

/* Timing */
#define PLAYBACK_THREAD_PRIORITY    50      /* RT priority */
#define MAIN_LOOP_SLEEP_MS          10      /* Main loop sleep */

/* Debug */
#define DEBUG_ENABLED               1       /* Enable debug prints */
#define DEBUG_FIFO_STATUS           0       /* Debug FIFO fill level */

/* ============================================================================
 * HELPER MACROS
 * ============================================================================ */

#if DEBUG_ENABLED
    #define DEBUG_PRINT(fmt, ...) \
        printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define INFO_PRINT(fmt, ...) \
    printf("[INFO] " fmt "\n", ##__VA_ARGS__)

/* Bit manipulation helpers */
#define SET_BIT(reg, bit)       ((reg) |= (1U << (bit)))
#define CLEAR_BIT(reg, bit)     ((reg) &= ~(1U << (bit)))
#define TOGGLE_BIT(reg, bit)    ((reg) ^= (1U << (bit)))
#define CHECK_BIT(reg, bit)     (((reg) >> (bit)) & 1U)

#endif /* CONFIG_H */
