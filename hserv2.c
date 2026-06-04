#define _GNU_SOURCE        // for getnameinfo()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// ------------------------------------------------------------------
//  Data structures (buffers enlarged to safe sizes)
// ------------------------------------------------------------------
#define MAX_PATH_LEN    1024


struct folder {
    char name[256];
    char path[MAX_PATH_LEN];
};

struct file {
    char name[256];
    char path[MAX_PATH_LEN];
    char sum;                      // unused, kept for compatibility
    long long len;
    long long cur;                 // offset into s.data
};

struct state {
    struct file   files[123];
    struct folder folders[123];
    char          data[123456];
    long long     data_cur;
    int           files_cur;
    int           folders_cur;
};
struct state s;

// ------------------------------------------------------------------
//  MIME type helper (unchanged)
// ------------------------------------------------------------------
char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
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

// ------------------------------------------------------------------
int main() {
    FILE* f;
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    long len;
    long long totalBytes;

    // --- scan store/ directory ---
    const char* storeDir = "store";
    dir = opendir(storeDir);
    if (!dir) {
        perror("opendir store");
        return 1;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        // build full path safely
        char fullPath[MAX_PATH_LEN];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", storeDir, ent->d_name);

        if (stat(fullPath, &st) != 0) {
            fprintf(stderr, "stat failed for %s\n", fullPath);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // directory
            strncpy(s.folders[s.folders_cur].name, ent->d_name, 256 - 1);
            strncpy(s.folders[s.folders_cur].path, storeDir, MAX_PATH_LEN - 1);
            s.folders_cur++;
            printf("[DIR]  %s P %s\n", ent->d_name, storeDir);
        }
        else {
            // regular file
            totalBytes = (long long)st.st_size;

            // store file info
            s.files[s.files_cur].len = totalBytes;
            s.files[s.files_cur].cur = s.data_cur;
            strncpy(s.files[s.files_cur].name, ent->d_name, 256 - 1);
            strncpy(s.files[s.files_cur].path, fullPath, MAX_PATH_LEN - 1);

            printf("[FILE] %s %lld p %s\n", ent->d_name, totalBytes, storeDir);

            // read file into s.data
            f = fopen(fullPath, "rb");
            if (f) {
                fseek(f, 0, SEEK_END);
                len = ftell(f);
                fseek(f, 0, SEEK_SET);
                if (s.data_cur + len <= sizeof(s.data)) {
                    fread(&s.data[s.data_cur], 1, len, f);
                    s.data_cur += len;
                }
                else {
                    fprintf(stderr, "Out of memory for file %s\n", fullPath);
                }
                fclose(f);
            }
            s.files_cur++;
        }
    }
    closedir(dir);

    // second sanitisation loop (original behaviour)
    for (int i = 0; i < s.data_cur; i++) {
        if (!s.data[i])      s.data[i] = 222;
        if (s.data[i] == 7)  s.data[i] = 223;
        if (s.data[i] == 27) s.data[i] = 221;
    }

    printf("SIZE %lld FL%i DIR%i\n", s.data_cur, s.files_cur, s.folders_cur);

    // print first 5 bytes of first 6 files (debug)
    for (int i = 0; i < 6 && i < s.files_cur; i++) {
        printf("%.5s\n", s.data + s.files[i].cur);
    }

    // ------------------------------------------------------------------
    //  HTTP server (Linux sockets)
    // ------------------------------------------------------------------
#define PORT            80
#define BUFFER_SIZE     4096

    int listen_sock, client_sock;
    struct sockaddr_in addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
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

    printf("HTTP server listening on port %d, serving 'store'\n", PORT);

    int resp = 0;
    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        char host[NI_MAXHOST], service[NI_MAXSERV];
        if (getnameinfo((struct sockaddr*)&client_addr, client_addr_len,
            host, sizeof(host), service, sizeof(service),
            NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
            printf("Connection from %s:%s\n", host, service);
        }

        char request[BUFFER_SIZE];
        ssize_t recv_len = recv(client_sock, request, sizeof(request) - 1, 0);
        if (recv_len > 0) {
            request[recv_len] = '\0';
            printf("asd!!! %i[%s]\n %s\n", resp, host, request);
        }

        // hard‑coded response (same as original)
        char header[] =
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 3483\r\n"
            "Server: snrv\r\n"
            "\r\n";
        send(client_sock, header, strlen(header), 0);

        // send the fourth file (index 3) if it exists
        if (s.files_cur > 3) {
            send(client_sock, &s.data[s.files[1].cur], 3483, 0);
        }

        close(client_sock);
        resp++;
    }

    close(listen_sock);
    return 0;
}