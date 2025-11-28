/**
 * @file wav_reader.c
 * @brief WAV file parser and reader implementation
 */

#include "wav_reader.h"
#include "config.h"
#include <stdio.h>
#include <string.h>



/* =======================
 * PRIVATE FUNCTIONS
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

    /* Calculate duration */
    wav->duration_seconds = wav->total_samples / wav->header.sample_rate;

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

    wav_print_info(wav);

    return 0;
}


void wav_close(wav_file_t *wav) {
    if (wav && wav->file) {
        fclose(wav->file);
        wav->file = NULL;
        DEBUG_PRINT("Closed WAV file: %s", wav->filename);
    }
}


int wav_read_samples(wav_file_t *wav, int16_t *buffer, uint32_t num_samples) {
    if (!wav || !wav->file || !buffer) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    /* Check EOF */
    if (wav->current_sample >= wav->total_samples) {
        return 0;
    }

    /* If less samples available than num_samples */
    uint32_t samples_to_read = num_samples;
    if (wav->current_sample + samples_to_read > wav->total_samples) {
        samples_to_read = wav->total_samples - wav->current_sample;
    }

    /* Read samples */
    size_t bytes_to_read = samples_to_read * (wav->header.bits_per_sample / 8) * wav->header.num_channels;

    size_t bytes_read = fread(buffer, 1, bytes_to_read, wav->file);

    if (bytes_read != bytes_to_read) {
        ERROR_PRINT("Read error: expected &zu bytes, got %zu", bytes_to_read, bytes_read);
        return -1;
    }


    /* If its stereo, convert to mono */
    if (wav->header.num_channels == 2) {
        for (uint32_t i = 0; i < samples_to_read; i++) {
            int32_t left = buffer[i * 2];
            int32_t right = buffer[i * 2 + 1];
            buffer[i] = (int16_t)((left + right) / 2);
        }
    }

    wav->current_sample += samples_to_read;

    return samples_to_read;
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

