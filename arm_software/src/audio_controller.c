/**
 * @file audio_controller.c
 * @brief Audio playback controller implementation
 */

#include "audio_controller.h"
#include "config.h"
#include "shared_mem_writer.h"
#include "audio_filter.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================ */

#define PLAYBACK_BUFFER_SAMPLES 1024

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Playback thread function
 */
static void* playback_thread_func(void *arg) {
    audio_controller_t *ctrl = (audio_controller_t *)arg;

    int16_t buffer[1024];

    INFO_PRINT("Playback thread started - NO TIMING MODE");

    while (ctrl->thread_running) {
        pthread_mutex_lock(&ctrl->state_mutex);

        if (ctrl->state == STATE_PLAYING && ctrl->current_wav.file != NULL) {
            pthread_mutex_unlock(&ctrl->state_mutex);

            /* Check buffer level */
            uint32_t available = shared_mem_get_available(ctrl->shmem);

            /* Solo escribir si buffer < 50% para evitar llenar demasiado */
            if (available < 32768) {  /* Menos de 50% (65536 / 2) */
                int samples_read = wav_read_samples(&ctrl->current_wav, buffer, 1024);

                if (samples_read > 0) {
                    /* Apply filter if enabled */
                    if (ctrl->filter_enabled && ctrl->filter.enabled) {
                        int16_t filtered_buffer[1024];
                        audio_filter_process_buffer(&ctrl->filter, buffer, filtered_buffer, (uint32_t)samples_read);
                        int written = shared_mem_write_samples(ctrl->shmem, filtered_buffer, (uint32_t)samples_read);
                        ctrl->samples_played += (uint32_t)written;
                    } else {
                        int written = shared_mem_write_samples(ctrl->shmem, buffer, (uint32_t)samples_read);
                        ctrl->samples_played += (uint32_t)written;
                    }

                    /* SIN SLEEP - dejar que loop corra libre */
                    /* NIOS controla rate completamente */
                } else if (samples_read == 0) {
                    INFO_PRINT("End of track, moving to next");
                    audio_controller_next(ctrl);
                } else {
                    ERROR_PRINT("Error reading WAV file");
                    audio_controller_stop(ctrl);
                }
            } else {
                /* Buffer > 50%, esperar que NIOS consuma */
                usleep(10000);  /* 10ms */
            }

        } else {
            pthread_mutex_unlock(&ctrl->state_mutex);
            usleep(50000);
        }
    }

    INFO_PRINT("Playback thread stopped");
    return NULL;
}

/**
 * @brief Check if file is a WAV file
 */
static int is_wav_file(const char *filename) {
    size_t len = strlen(filename);
    if (len < 4) return 0;
    return (strcmp(filename + len - 4, ".wav") == 0) ||
           (strcmp(filename + len - 4, ".WAV") == 0);
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

int audio_controller_init(audio_controller_t *ctrl, shared_mem_context_t *shmem) {
    if (!ctrl || !shmem) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(ctrl, 0, sizeof(audio_controller_t));

    ctrl->shmem = shmem;
    ctrl->state = STATE_STOPPED;
    ctrl->current_track = 0;
    ctrl->playlist_size = 0;
    ctrl->filter_enabled = 0;  /* Filter disabled by default */

    /* Initialize mutex */
    if (pthread_mutex_init(&ctrl->state_mutex, NULL) != 0) {
        ERROR_PRINT("Failed to initialize mutex");
        return -1;
    }

    /* Start playback thread */
    ctrl->thread_running = 1;
    if (pthread_create(&ctrl->playback_thread, NULL, playback_thread_func, ctrl) != 0) {
        ERROR_PRINT("Failed to create playback thread");
        pthread_mutex_destroy(&ctrl->state_mutex);
        return -1;
    }

    INFO_PRINT("Audio controller initialized");
    return 0;
}

void audio_controller_shutdown(audio_controller_t *ctrl) {
    if (!ctrl) return;

    INFO_PRINT("Shutting down audio controller");

    /* Stop playback thread */
    ctrl->thread_running = 0;
    pthread_join(ctrl->playback_thread, NULL);

    /* Close current WAV file */
    if (ctrl->current_wav.file) {
        wav_close(&ctrl->current_wav);
    }

    /* Destroy mutex */
    pthread_mutex_destroy(&ctrl->state_mutex);

    INFO_PRINT("Audio controller shutdown complete");
}

int audio_controller_load_playlist(audio_controller_t *ctrl, const char *directory) {
    if (!ctrl || !directory) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    DIR *dir = opendir(directory);
    if (!dir) {
        ERROR_PRINT("Failed to open directory: %s", directory);
        return -1;
    }

    ctrl->playlist_size = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && ctrl->playlist_size < MAX_PLAYLIST_SIZE) {
        if (is_wav_file(entry->d_name)) {
            /* Build full path */
            snprintf(ctrl->playlist[ctrl->playlist_size].filename,
                     sizeof(ctrl->playlist[ctrl->playlist_size].filename),
                     "%s/%s", directory, entry->d_name);

            /* Try to open to get metadata */
            wav_file_t temp_wav;
            if (wav_open(&temp_wav, ctrl->playlist[ctrl->playlist_size].filename) == 0) {
                strncpy(ctrl->playlist[ctrl->playlist_size].title, temp_wav.title, 63);
                strncpy(ctrl->playlist[ctrl->playlist_size].artist, temp_wav.artist, 63);
                strncpy(ctrl->playlist[ctrl->playlist_size].album, temp_wav.album, 63);
                ctrl->playlist[ctrl->playlist_size].duration = temp_wav.duration_seconds;
                wav_close(&temp_wav);

                INFO_PRINT("Added to playlist [%d]: %s",
                           ctrl->playlist_size,
                           ctrl->playlist[ctrl->playlist_size].title);

                ctrl->playlist_size++;
            }
        }
    }

    closedir(dir);

    INFO_PRINT("Loaded %d songs from %s", ctrl->playlist_size, directory);
    return ctrl->playlist_size;
}

int audio_controller_play(audio_controller_t *ctrl) {
    if (!ctrl) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (ctrl->state == STATE_PAUSED) {
        /* Resume from pause */
        ctrl->state = STATE_PLAYING;
        INFO_PRINT("Resumed playback");
        pthread_mutex_unlock(&ctrl->state_mutex);
        return 0;
    }

    if (ctrl->playlist_size == 0) {
        ERROR_PRINT("Playlist is empty");
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    /* Open current track */
    if (wav_open(&ctrl->current_wav, ctrl->playlist[ctrl->current_track].filename) != 0) {
        ERROR_PRINT("Failed to open track: %s", ctrl->playlist[ctrl->current_track].filename);
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    ctrl->state = STATE_PLAYING;
    ctrl->samples_played = 0;

    INFO_PRINT("Playing track %d: %s",
               ctrl->current_track,
               ctrl->playlist[ctrl->current_track].title);

    pthread_mutex_unlock(&ctrl->state_mutex);
    return 0;
}

void audio_controller_pause(audio_controller_t *ctrl) {
    if (!ctrl) return;

    pthread_mutex_lock(&ctrl->state_mutex);

    if (ctrl->state == STATE_PLAYING) {
        ctrl->state = STATE_PAUSED;
        INFO_PRINT("Playback paused");
    }

    pthread_mutex_unlock(&ctrl->state_mutex);
}

void audio_controller_stop(audio_controller_t *ctrl) {
    if (!ctrl) return;

    pthread_mutex_lock(&ctrl->state_mutex);

    ctrl->state = STATE_STOPPED;

    if (ctrl->current_wav.file) {
        wav_close(&ctrl->current_wav);
    }

    INFO_PRINT("Playback stopped");
    pthread_mutex_unlock(&ctrl->state_mutex);
}

int audio_controller_next(audio_controller_t *ctrl) {
    if (!ctrl) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (ctrl->playlist_size == 0) {
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    /* Close current file */
    if (ctrl->current_wav.file) {
        wav_close(&ctrl->current_wav);
    }

    /* Move to next track */
    ctrl->current_track++;
    if (ctrl->current_track >= ctrl->playlist_size) {
        ctrl->current_track = 0; /* Loop back to start */
    }

    /* If we were playing, start new track */
    if (ctrl->state == STATE_PLAYING) {
        pthread_mutex_unlock(&ctrl->state_mutex);
        return audio_controller_play(ctrl);
    }

    pthread_mutex_unlock(&ctrl->state_mutex);
    return 0;
}

int audio_controller_prev(audio_controller_t *ctrl) {
    if (!ctrl) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (ctrl->playlist_size == 0) {
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    /* Close current file */
    if (ctrl->current_wav.file) {
        wav_close(&ctrl->current_wav);
    }

    /* Move to previous track */
    if (ctrl->current_track == 0) {
        ctrl->current_track = ctrl->playlist_size - 1; /* Wrap to end */
    } else {
        ctrl->current_track--;
    }

    /* If we were playing, start new track */
    if (ctrl->state == STATE_PLAYING) {
        pthread_mutex_unlock(&ctrl->state_mutex);
        return audio_controller_play(ctrl);
    }

    pthread_mutex_unlock(&ctrl->state_mutex);
    return 0;
}

playback_state_t audio_controller_get_state(audio_controller_t *ctrl) {
    if (!ctrl) return STATE_STOPPED;

    pthread_mutex_lock(&ctrl->state_mutex);
    playback_state_t state = ctrl->state;
    pthread_mutex_unlock(&ctrl->state_mutex);

    return state;
}

const playlist_entry_t* audio_controller_get_current_track(audio_controller_t *ctrl) {
    if (!ctrl || ctrl->playlist_size == 0) {
        return NULL;
    }

    return &ctrl->playlist[ctrl->current_track];
}

void audio_controller_get_time(audio_controller_t *ctrl, uint32_t *current_sec, uint32_t *total_sec) {
    if (!ctrl) {
        if (current_sec) *current_sec = 0;
        if (total_sec) *total_sec = 0;
        return;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (current_sec) {
        *current_sec = wav_get_current_time(&ctrl->current_wav);
    }

    if (total_sec) {
        *total_sec = ctrl->current_wav.duration_seconds;
    }

    pthread_mutex_unlock(&ctrl->state_mutex);
}

int audio_controller_set_filter(audio_controller_t *ctrl, const filter_config_t *config) {
    if (!ctrl) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (config == NULL) {
        /* Disable filter */
        ctrl->filter_enabled = 0;
        INFO_PRINT("Filter disabled");
        pthread_mutex_unlock(&ctrl->state_mutex);
        return 0;
    }

    /* Initialize new filter */
    if (audio_filter_init(&ctrl->filter, config) != 0) {
        ERROR_PRINT("Failed to initialize filter");
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    ctrl->filter_enabled = 1;
    INFO_PRINT("Filter enabled");
    audio_filter_print_info(&ctrl->filter);

    pthread_mutex_unlock(&ctrl->state_mutex);
    return 0;
}

void audio_controller_enable_filter(audio_controller_t *ctrl, int enable) {
    if (!ctrl) return;

    pthread_mutex_lock(&ctrl->state_mutex);
    ctrl->filter_enabled = enable;
    audio_filter_enable(&ctrl->filter, enable);
    INFO_PRINT("Filter %s", enable ? "enabled" : "disabled");
    pthread_mutex_unlock(&ctrl->state_mutex);
}
