/**
 * @file audio_controller.h
 * @brief Audio playback controller
 *
 * Audio controller module
 * Manages sound playlists, play status and coordinates between WAV reader and FIFO
 */

#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include <stdint.h>
#include <pthread.h>

#include "fifo_writer.h"
#include "wav_reader.h"


/* ====================
 * PLAYBACK STATE
 * ==================== */

/**
 * @brief Playback states
 */
typedef enum { // TODO: For now I am declaring these 3 states only. I think that's it but let's see
    STATE_STOPPED,
    STATE_PLAYING,
    STATE_PAUSED
} playback_state_t;

/**
 * @brief Playlist entry
 */
typedef struct {
    char filename[256];
    char title[64];
    char artist[64];
    char album[64];
    uint32_t duration;
} playlist_entry_t;


/**
 * @brief Audio controller context
 */ 
typedef struct {
    /* Playlist */
    playlist_entry_t playlist[64]; /* Max 64 songs */
    int playlist_size; /* Actual size */
    int current_track;

    /* Playback state */
    playback_state_t state;
    wav_file_t current_wav;
    fifo_context_t *fifo;

    /* Threading */
    pthread_t playback_thread;
    pthread_mutex_t state_mutex;
    int thread_running;
} audio_controller_t;


/* ====================
 * PUBLIC API
 * ==================== */

/**
 * @brief Initialize audio controller
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param fifo Pointer to initialized FIFO context;
 * @return 0 on success, -1 on error
 */
int audio_controller_init(audio_controller_t *ctrl, fifo_context_t *fifo);


/**
 * @brief Shutdown audio controller
 *
 * Stops playback and cleans up resources
 *
 * @param ctrl Pointer to audio_controler_t structure
 */
void audio_controller_shutdown(audio_controller_t *ctrl);

/**
 * @brief Load playlist from directory
 *
 * Scans directory for WAV files and adds them to the playlist
 *
 * @param ctrl Pointer to audio_controller_t structure;
 * @param directory Path to music directory
 * @return Number of of songs loaded, -1 on error
 */
int audio_controller_load_playlist(audio_controller_t *ctrl, const char *directory);

/**
 * @brief Get current playback time
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param current_sec Output: current time in seconds
 * @param total_sec Output: total duration in seconds
 */
void audio_controller_get_time(audio_controller_t *ctrl, uint32_t *current_sec, uint32_t *total_sec);


#endif /* AUDIO_CONTROLLER_H */

