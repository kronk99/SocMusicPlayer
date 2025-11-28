/**
 * @file audio_controller.c
 * @brief Audio playback controller implementation
 */

#include "audio_controller.h"


/* ==============================
 * PRIVATE DEFINITIONS
 * ============================== */

#define PLAYBACK_BUFFER_SAMPLES 1024

/*
 * @brief
 */
static void* playback_thread_func(void *args) {
    audio_controller_t *ctrl = (audio_controller_t *)arg;
    int16_t buffer[PLAYBACK_BUFFER_SAMPLES];

    INFO_PRINT("Playback thread started");

    while (ctrl->thread_running) {
        pthread_mutex_lock(&ctrl->state_mutex);

        if (ctrl->state == STATE_PLAYING && ctrl->current_wav.file != NULL) {
            pthread_mutex_unlock(&ctrl->state_mutex);
            
            /* Read samples from WAV file */
            int samples_read = wav_read_samples(&ctrl->current_wav, buffer, PLAYBACK_BUFFER_SAMPLES);

            if (samples_read > 0) {
                /* Write to FIFO */
                int samples_written = fifo_write_samples(); // TODO: Doing this function
            } else if (samples_read == 0) {
                /* End of file, go to next track */
                INFO_PRINT("End of track, moving to next one...");
                audio_controller_next(ctrl); // TODO: Doing this function
            } else {
                /* Couldn't read file */
                ERROR_PRINT("Error reading WAV file");
                audio_controller_stop(ctrl); // TODO: Doing this function
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
