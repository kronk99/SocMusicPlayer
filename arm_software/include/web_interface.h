/**
 * @file web_interface.h
 * @brief Simple web interface for audio player
 *
 * Módulo para servir una interfaz web simple que muestra información
 * de la canción actual (artista, álbum, tiempo de reproducción).
 * Usa un servidor HTTP embebido ligero.
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <stdint.h>
#include <pthread.h>
#include "audio_controller.h"

/* ============================================================================
 * WEB SERVER CONFIGURATION
 * ============================================================================ */

#define WEB_MAX_CONNECTIONS     4
#define WEB_BUFFER_SIZE         4096
#define WEB_UPDATE_INTERVAL_MS  500     /* Update status every 500ms */

/**
 * @brief Web interface context
 */
typedef struct {
    audio_controller_t *audio_ctrl;     /* Reference to audio controller */

    /* Server state */
    int server_socket;
    int server_port;
    pthread_t server_thread;
    int thread_running;

    /* Content generation */
    char status_json[1024];             /* Current status as JSON */
    pthread_mutex_t status_mutex;
} web_interface_t;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Initialize web interface
 *
 * Starts HTTP server on specified port.
 *
 * @param web Pointer to web_interface_t structure
 * @param audio_ctrl Pointer to audio controller
 * @param port TCP port to listen on
 * @return 0 on success, -1 on error
 */
int web_interface_init(web_interface_t *web, audio_controller_t *audio_ctrl, int port);

/**
 * @brief Shutdown web interface
 *
 * Stops server and cleans up resources.
 *
 * @param web Pointer to web_interface_t structure
 */
void web_interface_shutdown(web_interface_t *web);

/**
 * @brief Update status information
 *
 * Called periodically to update the cached status.
 * This is thread-safe and can be called from any thread.
 *
 * @param web Pointer to web_interface_t structure
 */
void web_interface_update_status(web_interface_t *web);

/**
 * @brief Get current status as JSON string
 *
 * Returns a JSON string with current playback info.
 * Format:
 * {
 *   "state": "playing|paused|stopped",
 *   "track": "Song Title",
 *   "artist": "Artist Name",
 *   "album": "Album Name",
 *   "current_time": 123,
 *   "total_time": 240,
 *   "track_number": 3,
 *   "playlist_size": 10
 * }
 *
 * @param web Pointer to web_interface_t structure
 * @param buffer Output buffer for JSON string
 * @param buffer_size Size of output buffer
 * @return Number of bytes written
 */
int web_interface_get_status_json(web_interface_t *web, char *buffer, size_t buffer_size);

/**
 * @brief Generate HTML page
 *
 * Creates a simple HTML page with current playback information.
 *
 * @param web Pointer to web_interface_t structure
 * @param buffer Output buffer for HTML
 * @param buffer_size Size of output buffer
 * @return Number of bytes written
 */
int web_interface_generate_html(web_interface_t *web, char *buffer, size_t buffer_size);

#endif /* WEB_INTERFACE_H */
