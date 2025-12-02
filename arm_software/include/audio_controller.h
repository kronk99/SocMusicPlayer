/**
 * @file audio_controller.h
 * @brief Audio playback controller
 *
 * Módulo de control de reproducción de audio.
 * Maneja playlist, estado de reproducción, y coordina entre WAV reader y shared memory.
 */

#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include <stdint.h>
#include <pthread.h>
#include "wav_reader.h"
#include "shared_mem_writer.h"
#include "audio_filter.h"

/* ============================================================================
 * PLAYBACK STATE
 * ============================================================================ */

/**
 * @brief Playback states
 */
typedef enum {
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
    playlist_entry_t playlist[64];  /* Max 64 songs */
    int playlist_size;
    int current_track;

    /* Playback state */
    playback_state_t state;
    wav_file_t current_wav;
    shared_mem_context_t *shmem;

    /* Threading */
    pthread_t playback_thread;
    pthread_mutex_t state_mutex;
    int thread_running;

    /* Audio filter (optional) */
    audio_filter_t filter;
    int filter_enabled;

    /* Statistics */
    uint32_t samples_played;
    uint32_t buffer_underruns;
} audio_controller_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize audio controller
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param shmem Pointer to initialized shared memory context
 * @return 0 on success, -1 on error
 */
int audio_controller_init(audio_controller_t *ctrl, shared_mem_context_t *shmem);

/**
 * @brief Shutdown audio controller
 *
 * Stops playback and cleans up resources.
 *
 * @param ctrl Pointer to audio_controller_t structure
 */
void audio_controller_shutdown(audio_controller_t *ctrl);

/**
 * @brief Load playlist from directory
 *
 * Scans directory for WAV files and adds to playlist.
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param directory Path to music directory
 * @return Number of songs loaded, -1 on error
 */
int audio_controller_load_playlist(audio_controller_t *ctrl, const char *directory);

/**
 * @brief Start playback
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @return 0 on success, -1 on error
 */
int audio_controller_play(audio_controller_t *ctrl);

/**
 * @brief Pause playback
 *
 * @param ctrl Pointer to audio_controller_t structure
 */
void audio_controller_pause(audio_controller_t *ctrl);

/**
 * @brief Stop playback
 *
 * @param ctrl Pointer to audio_controller_t structure
 */
void audio_controller_stop(audio_controller_t *ctrl);

/**
 * @brief Go to next track
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @return 0 on success, -1 if at end of playlist
 */
int audio_controller_next(audio_controller_t *ctrl);

/**
 * @brief Go to previous track
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @return 0 on success, -1 if at beginning of playlist
 */
int audio_controller_prev(audio_controller_t *ctrl);

/**
 * @brief Get current playback state
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @return Current playback state
 */
playback_state_t audio_controller_get_state(audio_controller_t *ctrl);

/**
 * @brief Get current track information
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param track Output buffer for track info (can be NULL)
 * @return Pointer to current track entry, NULL if none
 */
const playlist_entry_t* audio_controller_get_current_track(audio_controller_t *ctrl);

/**
 * @brief Get current playback time
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param current_sec Output: current time in seconds
 * @param total_sec Output: total duration in seconds
 */
void audio_controller_get_time(audio_controller_t *ctrl, uint32_t *current_sec, uint32_t *total_sec);

/**
 * @brief Set audio filter
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param config Pointer to filter configuration (NULL to disable filter)
 * @return 0 on success, -1 on error
 */
int audio_controller_set_filter(audio_controller_t *ctrl, const filter_config_t *config);

/**
 * @brief Enable/disable audio filter
 *
 * @param ctrl Pointer to audio_controller_t structure
 * @param enable 1 to enable, 0 to disable
 */
void audio_controller_enable_filter(audio_controller_t *ctrl, int enable);

#endif /* AUDIO_CONTROLLER_H */
