/**
 * @file wav_reader.h
 * @brief WAV file parser and reader
 *
 * Module to read and parse WAV files
 * Supports 16-bit PCM, mono/stereo
 */

#ifndef WAV_READER_H
#define WAV_READER_H

/* ===============================
 * WAV FILE STRUCTURE
 * =============================== */

/**
 * @brief WAV file header structure (44 bytes standard - Typical PCM (Pulse Code Modulation))
 */
typedef struct {
    /* RIFF Header */
    char riff_tag[4];     /* "RIFF" */        // 1 byte * = 4 bytes 
    uint32_t riff_length; /* File size - 8 */ // 4 bytes 
    char wave_tag[4];     /* "WAVE" */        // 1 byte * = 4 bytes 

    /* Format chunk */
    char fmt_tag[4];          /* fmt " -> Stands for "Fragment/Chunk" */        // 1 byte * 4 = 4 bytes
    uint32_t fmt_length;      /* Format chunk size (16 for PCM) */              // 4 bytes
    uint16_t audio_format;    /* 1 = PCM */                                     // 2 bytes
    uint16_t num_channels;    /* 1 = Mono, 2 = Stereo */                        // 2 bytes
    uint32_t sample_rate;     /* Samples per second (We should use 48000 Hz) */ // 4 bytes
    uint32_t byte_rate;       /* Bytes per second */                            // 4 bytes
    uint16_t block_align;     /* Bytes per sample (all channels) */             // 2 bytes
    uint16_t bits_per_sample; /* Bits per sample */                             // 2 bytes
    
    /* Data chunk */
    char data_tag[4];         /* ""data" */                                     // 1 byte * 4 = 4 bytes
    uint32_t data_length;     /* Audio data size in bytes */                    // 4 bytes
} __attribute__((packed)) wav_header_t;


/**
 * @brief WAV file context for reading
 */
typedef struct {
    FILE *file;              /* File handle */
    wav_header_t header;     /* WAV header */
    uint32_t total_samples;  /* Total number of samples */
    uint32_t current_sample; /* Current playback position */
    char filename[256];      /* File path */

    /* Metadata (TODO: Should it be expanded? Only time would tell...) */
    char artist[64];
    char album[64];
    char title[64];
    uint32_t duration_seconds; /* Total duration */
} wav_file_t;



/* ============================
 * PUBLIC API
 * ============================ */

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
 * @brief Print WAV file information (debug)
 *
 * @param wav Pointer to wav_file_t structure
 */
void wav_print_info(wav_file_t *wav);




#endif /* WAV_READER_H */

