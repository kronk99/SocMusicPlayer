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


/* ====================
 * PLAYBACK STATE
 * ==================== */

/**
 * @brief PLaylist entry
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
    playlist_entry_t playlist[64] /* Max 64 songs */
    int playlist_size; /* Actual size */
    int current track;

    /* Threading */
    pthread_t playback_thread;
    int thread_running;
} audio_controller_t;





#endif /* AUDIO_CONTROLLER_H */
