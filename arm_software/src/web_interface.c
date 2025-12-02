/**
 * @file web_interface.c
 * @brief Simple web interface implementation
 */

#include "web_interface.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================ */

/**
 * @brief Handle HTTP request
 */
static void handle_http_request(web_interface_t *web, int client_sock) {
    char buffer[WEB_BUFFER_SIZE];
    char response[WEB_BUFFER_SIZE];

    /* Read request (we don't parse it, just respond) */
    recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    /* Generate HTML response */
    int content_len = web_interface_generate_html(web, buffer, sizeof(buffer));

    /* Build HTTP response */
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             content_len, buffer);

    /* Send response */
    send(client_sock, response, strlen(response), 0);
    close(client_sock);
}

/**
 * @brief Web server thread
 */
static void* server_thread_func(void *arg) {
    web_interface_t *web = (web_interface_t *)arg;

    INFO_PRINT("Web server thread started on port %d", web->server_port);

    while (web->thread_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        /* Accept connection (with timeout) */
        int client_sock = accept(web->server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock >= 0) {
            DEBUG_PRINT("Web connection from %s", inet_ntoa(client_addr.sin_addr));

            /* Update status before serving */
            web_interface_update_status(web);

            /* Handle request */
            handle_http_request(web, client_sock);
        }

        usleep(10000); /* Small delay */
    }

    INFO_PRINT("Web server thread stopped");
    return NULL;
}

/* ============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================ */

int web_interface_init(web_interface_t *web, audio_controller_t *audio_ctrl, int port) {
    if (!web || !audio_ctrl) {
        ERROR_PRINT("Invalid parameters");
        return -1;
    }

    memset(web, 0, sizeof(web_interface_t));

    web->audio_ctrl = audio_ctrl;
    web->server_port = port;

    /* Initialize mutex */
    if (pthread_mutex_init(&web->status_mutex, NULL) != 0) {
        ERROR_PRINT("Failed to initialize mutex");
        return -1;
    }

    /* Create socket */
    web->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (web->server_socket < 0) {
        ERROR_PRINT("Failed to create socket");
        pthread_mutex_destroy(&web->status_mutex);
        return -1;
    }

    /* Set socket options */
    int opt = 1;
    setsockopt(web->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(web->server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ERROR_PRINT("Failed to bind to port %d", port);
        close(web->server_socket);
        pthread_mutex_destroy(&web->status_mutex);
        return -1;
    }

    /* Listen */
    if (listen(web->server_socket, 5) < 0) {
        ERROR_PRINT("Failed to listen");
        close(web->server_socket);
        pthread_mutex_destroy(&web->status_mutex);
        return -1;
    }

    /* Start server thread */
    web->thread_running = 1;
    if (pthread_create(&web->server_thread, NULL, server_thread_func, web) != 0) {
        ERROR_PRINT("Failed to create server thread");
        close(web->server_socket);
        pthread_mutex_destroy(&web->status_mutex);
        return -1;
    }

    INFO_PRINT("Web interface initialized on port %d", port);
    return 0;
}

void web_interface_shutdown(web_interface_t *web) {
    if (!web) return;

    INFO_PRINT("Shutting down web interface");

    web->thread_running = 0;
    close(web->server_socket);
    pthread_join(web->server_thread, NULL);

    pthread_mutex_destroy(&web->status_mutex);

    INFO_PRINT("Web interface shutdown complete");
}

void web_interface_update_status(web_interface_t *web) {
    if (!web || !web->audio_ctrl) return;

    pthread_mutex_lock(&web->status_mutex);

    playback_state_t state = audio_controller_get_state(web->audio_ctrl);
    const playlist_entry_t *track = audio_controller_get_current_track(web->audio_ctrl);

    uint32_t current_sec, total_sec;
    audio_controller_get_time(web->audio_ctrl, &current_sec, &total_sec);

    /* Build JSON status */
    const char *state_str = (state == STATE_PLAYING) ? "playing" :
                            (state == STATE_PAUSED) ? "paused" : "stopped";

    snprintf(web->status_json, sizeof(web->status_json),
             "{"
             "\"state\":\"%s\","
             "\"track\":\"%s\","
             "\"artist\":\"%s\","
             "\"album\":\"%s\","
             "\"current_time\":%u,"
             "\"total_time\":%u"
             "}",
             state_str,
             track ? track->title : "N/A",
             track ? track->artist : "N/A",
             track ? track->album : "N/A",
             current_sec,
             total_sec);

    pthread_mutex_unlock(&web->status_mutex);
}

int web_interface_get_status_json(web_interface_t *web, char *buffer, size_t buffer_size) {
    if (!web || !buffer) return 0;

    pthread_mutex_lock(&web->status_mutex);
    int len = snprintf(buffer, buffer_size, "%s", web->status_json);
    pthread_mutex_unlock(&web->status_mutex);

    return len;
}

int web_interface_generate_html(web_interface_t *web, char *buffer, size_t buffer_size) {
    if (!web || !buffer) return 0;

    playback_state_t state = audio_controller_get_state(web->audio_ctrl);
    const playlist_entry_t *track = audio_controller_get_current_track(web->audio_ctrl);

    uint32_t current_sec, total_sec;
    audio_controller_get_time(web->audio_ctrl, &current_sec, &total_sec);

    const char *state_str = (state == STATE_PLAYING) ? "▶ Playing" :
                            (state == STATE_PAUSED) ? "⏸ Paused" : "⏹ Stopped";

    int len = snprintf(buffer, buffer_size,
        "<!DOCTYPE html>\n"
        "<html><head><title>SoC Audio Player</title>\n"
        "<meta http-equiv=\"refresh\" content=\"2\">\n"
        "<style>\n"
        "body{font-family:Arial;margin:40px;background:#f0f0f0}\n"
        ".container{background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}\n"
        "h1{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:10px}\n"
        ".status{font-size:24px;margin:20px 0;color:#4CAF50}\n"
        ".info{font-size:18px;margin:10px 0}\n"
        ".time{font-size:20px;font-weight:bold;color:#2196F3;margin:20px 0}\n"
        "</style>\n"
        "</head><body>\n"
        "<div class=\"container\">\n"
        "<h1>🎵 SoC Audio Player</h1>\n"
        "<div class=\"status\">%s</div>\n"
        "<div class=\"info\"><strong>Track:</strong> %s</div>\n"
        "<div class=\"info\"><strong>Artist:</strong> %s</div>\n"
        "<div class=\"info\"><strong>Album:</strong> %s</div>\n"
        "<div class=\"time\">⏱ Time: %02u:%02u / %02u:%02u</div>\n"
        "<p><em>Page auto-refreshes every 2 seconds</em></p>\n"
        "</div>\n"
        "</body></html>",
        state_str,
        track ? track->title : "No track loaded",
        track ? track->artist : "Unknown",
        track ? track->album : "Unknown",
        current_sec / 60, current_sec % 60,
        total_sec / 60, total_sec % 60);

    return len;
}
