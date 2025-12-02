/**
 * @file wav_reader.h
 * @brief WAV file parser and reader
 *
 * Módulo para leer y parsear archivos WAV desde SD card.
 * Soporta formato PCM 16-bit, mono/stereo.
 */

#ifndef WAV_READER_H
#define WAV_READER_H

#include <stdint.h>
#include <stdio.h>

/* ============================================================================
 * WAV FILE STRUCTURES
 * ============================================================================ */

/**
 * @brief WAV file header structure (44 bytes standard - Typical PCM (Pulse Code Modulation))
 */
typedef struct {
    /* RIFF Header */
    char riff_tag[4];           /* "RIFF" */          // 1 byte * 4 = 4 bytes
    uint32_t riff_length;       /* File size - 8 */   // 4 bytes
    char wave_tag[4];           /* "WAVE" */          // 1 byte * 4 = 4 bytes

    /* Format Chunk */
    char fmt_tag[4];            /* "fmt " -> Stands for "Fragment/Chunk" */   // 1 byte * 4 = 4 bytes
    uint32_t fmt_length;        /* Format chunk size (16 for PCM) */          // 4 bytes
    uint16_t audio_format;      /* 1 = PCM */                                 // 2 bytes
    uint16_t num_channels;      /* 1 = Mono, 2 = Stereo */                    // 2 bytes
    uint32_t sample_rate;       /* Samples per second (We should use 48 k) */ // 4 bytes
    uint32_t byte_rate;         /* Bytes per second */                        // 4 bytes
    uint16_t block_align;       /* Bytes per sample (all channels) */         // 2 bytes
    uint16_t bits_per_sample;   /* Bits per sample */                         // 2 bytes

    /* Data Chunk */
    char data_tag[4];           /* "data" */                                  // 1 byte * 4 = 4 bytes
    uint32_t data_length;       /* Audio data size in bytes */                // 4 bytes
} __attribute__((packed)) wav_header_t;

/**
 * @brief WAV file context for reading
 */
typedef struct {
    FILE *file;                 /* File handle */
    wav_header_t header;        /* WAV header */
    uint32_t total_samples;     /* Total number of samples */
    uint32_t current_sample;    /* Current playback position */
    char filename[256];         /* File path */

    /* Metadata (puede expandirse) */
    char artist[64];
    char album[64];
    char title[64];
    uint32_t duration_seconds;  /* Total duration */
} wav_file_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Open and parse a WAV file
 *
 * @param wav Pointer to wav_file_t structure
 * @param filename Path to WAV file
 * @return 0 on success, -1 on error
 */
int wav_open(wav_file_t *wav, const char *filename);

/**
 * @brief Close WAV file
 *
 * @param wav Pointer to wav_file_t structure
 */
void wav_close(wav_file_t *wav);

/**
 * @brief Read audio samples from WAV file
 *
 * @param wav Pointer to wav_file_t structure
 * @param buffer Buffer to store samples (int16_t array)
 * @param num_samples Number of samples to read
 * @return Number of samples actually read, 0 on EOF, -1 on error
 */
int wav_read_samples(wav_file_t *wav, int16_t *buffer, uint32_t num_samples);

/**
 * @brief Seek to specific sample position
 *
 * @param wav Pointer to wav_file_t structure
 * @param sample_position Sample number to seek to
 * @return 0 on success, -1 on error
 */
int wav_seek(wav_file_t *wav, uint32_t sample_position);

/**
 * @brief Reset to beginning of file
 *
 * @param wav Pointer to wav_file_t structure
 * @return 0 on success, -1 on error
 */
int wav_rewind(wav_file_t *wav);

/**
 * @brief Get current playback time in seconds
 *
 * @param wav Pointer to wav_file_t structure
 * @return Current time in seconds
 */
uint32_t wav_get_current_time(wav_file_t *wav);

/**
 * @brief Check if WAV file format is supported
 *
 * @param wav Pointer to wav_file_t structure
 * @return 1 if supported, 0 if not
 */
int wav_is_format_supported(wav_file_t *wav);

/**
 * @brief Print WAV file information (debug)
 *
 * @param wav Pointer to wav_file_t structure
 */
void wav_print_info(wav_file_t *wav);

#endif /* WAV_READER_H */
