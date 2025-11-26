// servidor http minimo para exponer metadatos de audio  
// usa sockets y responde solo a la ruta get /api/track  

#include <stdio.h>      
#include <stdlib.h>       
#include <string.h>     
#include <stdint.h>       
#include <unistd.h>     // close, read, write  
#include <errno.h>      // manejo de errores  
#include <arpa/inet.h>  // funciones para sockets tcp  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h> // estructuras para redes  

#define SERVER_PORT 8080   // puerto tcp donde escucha el servidor  
#define BACKLOG     4      // cantidad maxima de conexiones en cola  
#define RECV_BUF    2048   // tamano del buffer de recepcion  
#define RESP_BUF    2048   // tamano del buffer de respuesta  

// estructura que modela la informacion de una pista de audio  
// este formato coincide con el usado en el codigo wav reader del hps  
typedef struct {
    char     filename[256];    // nombre del archivo wav  
    char     title[64];        // titulo de la cancion  
    char     artist[64];       // artista  
    char     album[64];        // album  
    uint32_t duration_seconds; // duracion total  
    uint32_t current_seconds;  // tiempo actual de reproduccion  
    uint32_t sample_rate;      // frecuencia de muestreo  
    uint16_t num_channels;     // cantidad de canales  
    uint16_t bits_per_sample;  // bits por muestra  
} track_info_t;

// datos quemados para pruebas mientras no se integre el wav_reader  
// esto permite validar toda la ruta de comunicacion con el cliente web  
static track_info_t g_track = {
    .filename        = "music/example_song.wav",
    .title           = "Cancion Ejemplito",
    .artist          = "Artista desconocido",
    .album           = "Demo Album",
    .duration_seconds = 245,   // ejemplo  
    .current_seconds  = 73,    // ejemplo  
    .sample_rate      = 48000,
    .num_channels     = 2,
    .bits_per_sample  = 16
};

// garantiza el envio completo de un buffer  
// send podria enviar menos bytes, asi que se repite hasta completar  
static int send_all(int fd, const char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(fd, buf + total, len - total, 0);
        if (n <= 0) {
            return -1; // error al enviar  
        }
        total += (size_t)n;
    }
    return 0;
}

// maneja la ruta /api/track  
// prepara un json con los metadatos y lo envia con cabeceras http  
static void handle_api_track(int client_fd) {
    char json[RESP_BUF];

    // construccion del json segun campos del track  
    int n = snprintf(
        json, sizeof(json),
        "{"
        "\"filename\":\"%s\","
        "\"title\":\"%s\","
        "\"artist\":\"%s\","
        "\"album\":\"%s\","
        "\"duration_seconds\":%u,"
        "\"current_seconds\":%u,"
        "\"sample_rate\":%u,"
        "\"channels\":%u,"
        "\"bits_per_sample\":%u"
        "}\n",
        g_track.filename,
        g_track.title,
        g_track.artist,
        g_track.album,
        g_track.duration_seconds,
        g_track.current_seconds,
        g_track.sample_rate,
        (unsigned)g_track.num_channels,
        (unsigned)g_track.bits_per_sample
    );

    // validar si hubo error o truncamiento  
    if (n < 0 || (size_t)n >= sizeof(json)) {
        const char *err =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send_all(client_fd, err, strlen(err));
        return;
    }

    // preparar header http  
    char header[256];
    int h = snprintf(
        header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        n
    );

    // validar header  
    if (h < 0 || (size_t)h >= sizeof(header)) {
        const char *err =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send_all(client_fd, err, strlen(err));
        return;
    }

    // enviar header y json  
    send_all(client_fd, header, (size_t)h);
    send_all(client_fd, json, (size_t)n);
}

// maneja rutas no encontradas  
static void handle_not_found(int client_fd) {
    const char *body = "Not Found\n";
    char header[256];

    int h = snprintf(
        header, sizeof(header),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        strlen(body)
    );

    send_all(client_fd, header, (size_t)h);
    send_all(client_fd, body, strlen(body));
}

// procesa una conexion de cliente  
// lee la primera linea de la peticion y decide que ruta hacer  
static void handle_client(int client_fd) {
    char buffer[RECV_BUF];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    // si no se recibio nada se finaliza  
    if (n <= 0) {
        return;
    }

    buffer[n] = '\0'; // terminacion de cadena  

    // extraer metodo http y ruta  
    char method[8] = {0};
    char path[128] = {0};

    // se espera algo como get /api/track http/numero
    if (sscanf(buffer, "%7s %127s", method, path) != 2) {
        return;
    }

    // solo se permite get  
    if (strcmp(method, "GET") != 0) {
        const char *resp =
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send_all(client_fd, resp, strlen(resp));
        return;
    }

    // ruta valida  
    if (strcmp(path, "/api/track") == 0) {
        handle_api_track(client_fd);
    } else {
        handle_not_found(client_fd);
    }
}

int main(void) {
    int server_fd;
    struct sockaddr_in addr;
    int opt = 1;

    // crear socket tcp  
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // permitir reutilizar el puerto  
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    // configurar direccion de escucha  aaaaaaaaaa
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // acepta desde cualquier ip  
    addr.sin_port        = htons(SERVER_PORT);

    // asociar socket al puerto  
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // iniciar escucha  
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Audio meta server listening on port %d\n", SERVER_PORT);

    // ciclo infinito para aceptar clientes  
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // aceptar conexion entrante  
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // procesar peticion  
        handle_client(client_fd);

        // cerrar conexion  
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
