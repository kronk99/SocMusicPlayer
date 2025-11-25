/**
 * @file wav_reader.c
 * @brief WAV file parser and reader implementation
 */

#include "wav_reader.h"
#include "config.h"
#include <stdio.h>
#include <string.h>



/* =======================
 * PUBLIC FUNCTIONS
 * ======================= */

/**
 * @brief Extract metadata from filename (simple implementation)
 */
static void extract_metadata(wav_file_t *wav) {
    const char *basename = strrchr(wav->filename, '/');
    if (basename)
        basename++;
    else
        basename = wav->filename;

    /* Copy 63 chars and leave last one for NULL terminator */
    strncpy(wav->title, basename, sizeof(wav->title) - 1);
    wav -> title[sizeof(wav->title) - 1] = '\0';

    /*  Remove the extension */
    char *dot = strrchr(wav->title, '.');
    if (dot && strcmp(dot, ".wav") == 0)
        *dot = '\0';

    // TODO: Get real values
    strncpy(wav->artist, "Unknown Artist", sizeof(wav->artist) - 1);
    strncpy(wav->album, "Unknown Album", sizeof(wav->album) - 1);
}


/* =======================
 * PUBLIC FUNCTIONS
 * ======================= */
int wav_open(wav_file_t *wav, const char *filename) {
    if (!wav || !filename) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(wav, 0, sizeof(wav_file_t));

    /* Open file*/
    wav->file = fopen(filename, "rb");
    if (!wav->file) {
        ERROR_PRINT("Failed to open file: %s", filename);
        return -1;
    }

    /* Store filename */
    strncpy(wav->filename, filename, sizeof(wav->filename) - 1);

    /* Read WAV header */
    size_t read = fread(&wav->header, 1, sizeof(wav_header_t), wav->file);
    if (read != sizeof(wav_header_t)) {
        ERROR_PRINT("Failed to read WAV header");
        fclose(wav->file);
        return -1;
    }


    /* Calculate total samples */
    wav->total_samples = wav->header.data_length / (wav->header.bits_per_sample / 8) / wav->header.num_channels;

    /* Extract metadata */
    extract_metadata(wav);

    /* Reset playback position */
    wav->current_sample = 0;

    INFO_PRINT("Opened WAV file: %s", filename);
    DEBUG_PRINT("  Sample rate: %d Hz", wav->header.sample_rate);
    DEBUG_PRINT("  Channels: %d", wav->header.num_channels);
    DEBUG_PRINT("  Bits per sample: %d", wav->header.bits_per_sample);
    DEBUG_PRINT("  Total samples: %u", wav->total_samples);
    DEBUG_PRINT("  Duration: %u seconds", wav->duration_seconds);

    return 0;
}


void wav_close(wav_file_t *wav) {
    if (wav && wav->file) {
        fclose(wav->file);
        wav->file = NULL;
        DEBUG_PRINT("Closed WAV file: %s", wav->filename);
    }
}

void wav_print_info(wav_file_t *wav) {
    if (!wav) {
        return;
    }

    printf("============================\n");
    printf("    WAV File Information    \n");
    printf("============================\n");
    printf("Filename: %s\n", wav->filename);
    printf("Title:    %s\n", wav->title);
    printf("Artist:   %s\n", wav->artist);
    printf("Album:    %s\n", wav->album);
    printf("----------------------------\n");
    printf("Sample Rate:   %u Hz\n", wav->header.sample_rate);
    printf("Channels:      %u\n", wav->header.num_channels);
    printf("Bits/Sample:   %u\n", wav->header.bits_per_sample);
    printf("Total samples: %u\n", wav->total_samples);
    printf("Duration:      %u:%02u\n", wav->duration_seconds/60, wav->duration_seconds%60);
    printf("Data size:     %u bytes\n", wav->header.data_length);
    printf("============================\n");
}

