#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT            80
#define ROOT_DIR        "www"          /* relative or absolute path */
#define BUFFER_SIZE     4096
#define MAX_PATH_LEN    1024

/* Simple MIME type lookup */
const char* get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0)  return "text/css";
    if (strcmp(ext, ".js") == 0)   return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)  return "image/gif";
    if (strcmp(ext, ".svg") == 0)  return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
    if (strcmp(ext, ".txt") == 0)  return "text/plain";
    if (strcmp(ext, ".pdf") == 0)  return "application/pdf";
    if (strcmp(ext, ".zip") == 0)  return "application/zip";
    return "application/octet-stream";
}

/* Send an error response (404, 500, etc.) */
void send_error(int client, int code, const char *message) {
    char header[512];
    char body[256];
    snprintf(body, sizeof(body), "<html><body><h1>%d %s</h1></body></html>", code, message);
    snprintf(header, sizeof(header),
             "HTTP/1.0 %d %s\r\n"
             "Content-Type: text/html\r\n"
             "Connection: close\r\n"
             "Server:  sn~><?\r\n"
             "Content-Length: %zu\r\n"
             "\r\n",
             code, message, strlen(body));
    send(client, header, strlen(header), 0);
    send(client, body, strlen(body), 0);
}

/* Safely read a file into memory (returns NULL on failure) */
char* read_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char*)malloc(len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, len, f);
    fclose(f);
    buf[len] = '\0';
    if (out_len) *out_len = len;
    return buf;
}

/* Handle one HTTP request */
void handle_client(int client) {
    char request[BUFFER_SIZE];
    ssize_t recv_len = recv(client, request, sizeof(request) - 1, 0);
    if (recv_len <= 0) {
        close(client);
        return;
    }
    request[recv_len] = '\0';

    /* Parse request line: GET /path HTTP/1.x */
    char method[16], path[MAX_PATH_LEN];
    if (sscanf(request, "%15s %1023s", method, path) != 2) {
        send_error(client, 400, "Bad Request");
        close(client);
        return;
    }

    /* Only GET is supported */
    if (strcmp(method, "GET") != 0) {
        send_error(client, 501, "Not Implemented");
        close(client);
        return;
    }

    /* Decode %XX and prevent directory traversal */
    char decoded[MAX_PATH_LEN];
    int j = 0;
    for (int i = 0; path[i] && j < sizeof(decoded) - 1; i++) {
        if (path[i] == '%' && path[i+1] && path[i+2]) {
            int hex;
            if (sscanf(path + i + 1, "%2x", &hex) == 1) {
                decoded[j++] = (char)hex;
                i += 2;
                continue;
            }
        }
        decoded[j++] = path[i];
    }
    decoded[j] = '\0';

    /* Block directory traversal */
    if (strstr(decoded, "..") != NULL) {
        send_error(client, 403, "Forbidden");
        close(client);
        return;
    }

    /* Map '/' to index.html */
    if (strcmp(decoded, "/") == 0)
        strcpy(decoded, "/index.html");

    /* Build full filesystem path */
    char filepath[MAX_PATH_LEN];
    snprintf(filepath, sizeof(filepath), "%s%s", ROOT_DIR, decoded);

    /* Read and serve file */
    long file_len = 0;
    char *file_content = read_file(filepath, &file_len);
    if (!file_content) {
        send_error(client, 404, "Not Found");
        close(client);
        return;
    }

    /* Construct success header */
    const char *mime = get_mime_type(filepath);
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.0 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
                     "Server:  sn~><?\r\n"
             "Connection: close\r\n"
             "\r\n",
             mime, file_len);

    send(client, header, strlen(header), 0);
    send(client, file_content, file_len, 0);
    free(file_content);
    close(client);
}

struct sockaddr_in client_addr;

int main() {
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  /* avoid "address already in use" */

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_sock);
        return 1;
    }

    if (listen(listen_sock, SOMAXCONN) < 0) {
        perror("listen");
        close(listen_sock);
        return 1;
    }

    printf("HTTP server listening on port %d, serving '%s'\n", PORT, ROOT_DIR);

    while (1) {
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listen_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client < 0) {
            perror("accept");
            continue;
        }
        handle_client(client);
    }

    close(listen_sock);
    return 0;
}