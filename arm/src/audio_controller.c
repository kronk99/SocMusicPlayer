/**
 * @file audio_controller.c
 * @brief Audio playback controller implementation
 */

#include "audio_controller.h"
#include "config.h"
#include <string.h>
#include <dirent.h>

/* ==============================
 * PRIVATE DEFINITIONS
 * ============================== */

#define PLAYBACK_BUFFER_SAMPLES 1024


/* ==============================
 * PRIVATE FUNCTIONS
 * ============================== */

/*
 * @brief Playback thread function
 */
static void* playback_thread_func(void *args) {
    audio_controller_t *ctrl = (audio_controller_t *)args;
    int16_t buffer[PLAYBACK_BUFFER_SAMPLES];

    INFO_PRINT("Playback thread started");

    while (ctrl->thread_running) {
        pthread_mutex_lock(&ctrl->state_mutex);

        if (/*ctrl->state == STATE_PLAYING && */ctrl->current_wav.file != NULL) {
            pthread_mutex_unlock(&ctrl->state_mutex);
            
            /* Read samples from WAV file */
            int samples_read = wav_read_samples(&ctrl->current_wav, buffer, PLAYBACK_BUFFER_SAMPLES);

            if (samples_read > 0) {
                /* Write to FIFO */
                int samples_written = fifo_write_samples(ctrl->fifo, buffer, samples_read);

                if (samples_written < samples_read) {
                    /* FIFO is full, wait a bit */
                    DEBUG_PRINT("FIFO full, wrote %d/%d samples", samples_written, samples_read);
                    usleep(10000) /* 10 ms */;
                } 
            } else if (samples_read == 0) {
                /* End of file, go to next track */
                INFO_PRINT("End of track, moving to next one...");
                audio_controller_next(ctrl);
            } else {
                /* Couldn't read file */
                ERROR_PRINT("Error reading WAV file");
                audio_controller_stop(ctrl);
            }

        } else {
            pthread_mutex_unlock(&ctrl->state_mutex);
            /* Not playing, sleep */
            usleep(50000); /* 50 ms */
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
    if (len < 4) return 0; // Does not fit the extension
    return (strcmp(filename + len - 4, ".wav") == 0) ||
           (strcmp(filename + len - 4, ".WAV") == 0);
}


/* ==============================
 * PUBLIC DEFINITIONS
 * ============================== */
int audio_controller_init(audio_controller_t *ctrl, fifo_context_t *fifo) {
    if (!ctrl || !fifo)  {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(ctrl, 0, sizeof(audio_controller_t));

    ctrl->fifo = fifo;
    ctrl->state = STATE_STOPPED;
    ctrl->current_track = 0;
    ctrl->playlist_size = 0;

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

}

void audio_controller_get_time(audio_controller_t *ctrl, uint32_t *current_sec, uint32_t *total_sec) {

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
   return 0;
}

int audio_controller_play(audio_controller_t *ctrl) {
    if (!ctrl) {
        ERROR_PRINT("Invalid paramters");
        return -1;
    }

    pthread_mutex_lock(&ctrl->state_mutex);

    if (ctrl->playlist_size == 0) {
        ERROR_PRINT("PLaylist is empty");
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    /* Open current track */
    if (wav_open(&ctrl->current_wav, ctrl->playlist[ctrl->current_track].filename) != 0) {
        ERROR_PRINT("Failed to open track: %s", ctrl->playlist[ctrl->current_track].filename);
        pthread_mutex_unlock(&ctrl->state_mutex);
        return -1;
    }

    INFO_PRINT("Playing track: %d: %s", ctrl->current_track, ctrl->playlist[ctrl->current_track].title);

    pthread_mutex_unlock(&ctrl->state_mutex);
    return 0;
}

void audio_controller_stop(audio_controller_t *ctrl) {

}

void audio_controller_next(audio_controller_t *ctrl) {

}

