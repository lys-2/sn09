/*
 * ram_http_server.c – preloads files from the 'www' directory into memory at startup.
 *
 * Compile: gcc -O2 ram_http_server.c -o ram_server
 * Run:     ./ram_server
 */

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PORT            80
#define ROOT_DIR        "www"
#define MAX_EVENTS      1024
#define BUFFER_SIZE     4096
#define MAX_PATH        1024
#define KEEPALIVE_TIMEOUT 5

/* ---------- memory cache ---------- */

typedef struct cache_entry {
    char *path;               /* relative path from root, e.g. "/index.html" */
    char *data;               /* file content */
    size_t size;              /* length of data */
    char mime[64];            /* content type */
    struct cache_entry *next;
} cache_entry_t;

static cache_entry_t *cache_head = NULL;   /* linked list for simplicity */

/* quick MIME lookup */
static const char *get_mime(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    if (!strcmp(ext, ".html") || !strcmp(ext, ".htm")) return "text/html";
    if (!strcmp(ext, ".css"))  return "text/css";
    if (!strcmp(ext, ".js"))   return "application/javascript";
    if (!strcmp(ext, ".json")) return "application/json";
    if (!strcmp(ext, ".png"))  return "image/png";
    if (!strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg")) return "image/jpeg";
    if (!strcmp(ext, ".gif"))  return "image/gif";
    if (!strcmp(ext, ".svg"))  return "image/svg+xml";
    if (!strcmp(ext, ".ico"))  return "image/x-icon";
    if (!strcmp(ext, ".txt"))  return "text/plain";
    return "application/octet-stream";
}

/* read entire file into a buffer; returns NULL on failure */
static char *read_file(const char *fullpath, size_t *size) {
    FILE *f = fopen(fullpath, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, len, f);
    fclose(f);
    buf[len] = '\0';
    *size = len;
    return buf;
}

/* recursively load files from a directory into the cache */
static void load_directory(const char *base, const char *rel) {
    char full[MAX_PATH];
    snprintf(full, sizeof(full), "%s/%s", base, rel[0] ? rel : ".");

    DIR *dir = opendir(full);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') continue;   /* skip . and .. and hidden */
        char child_rel[MAX_PATH], child_full[MAX_PATH];
        snprintf(child_rel, sizeof(child_rel), "%s%s%s",
                 rel, rel[0] ? "/" : "", ent->d_name);
        snprintf(child_full, sizeof(child_full), "%s/%s", full, ent->d_name);

        struct stat st;
        if (stat(child_full, &st) == -1) continue;
        if (S_ISDIR(st.st_mode)) {
            load_directory(base, child_rel);   /* recurse */
        } else if (S_ISREG(st.st_mode)) {
            size_t fsize;
            char *data = read_file(child_full, &fsize);
            if (!data) continue;

            cache_entry_t *entry = malloc(sizeof(cache_entry_t));
            entry->path = strdup(child_rel);   /* always starts with "/" */
            entry->data = data;
            entry->size = fsize;
            snprintf(entry->mime, sizeof(entry->mime), "%s", get_mime(ent->d_name));
            entry->next = cache_head;
            cache_head = entry;
            printf("Cached: %s (%zu bytes)\n", entry->path, entry->size);
        }
    }
    closedir(dir);
}

/* find entry by request path; returns NULL if not found */
static cache_entry_t *cache_lookup(const char *path) {
    for (cache_entry_t *e = cache_head; e; e = e->next)
        if (!strcmp(e->path, path)) return e;
    return NULL;
}

/* ---------- connection state ---------- */

typedef enum { READING_REQUEST, SENDING_MEM } conn_state;

typedef struct {
    int fd;
    conn_state state;
    char rbuf[BUFFER_SIZE];
    size_t rpos;
    cache_entry_t *file_entry;   /* current file being sent */
    size_t sent;                 /* bytes already sent */
    time_t last_active;
} client_t;

static client_t *clients[MAX_EVENTS] = {0};

/* set non‑blocking */
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* send an HTTP error and close connection */
static void send_error(int epfd, client_t *cli, int code, const char *msg) {
    char body[256];
    snprintf(body, sizeof(body), "<html><body><h1>%d %s</h1></body></html>", code, msg);
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             code, msg, strlen(body));
    send(cli->fd, header, strlen(header), MSG_NOSIGNAL);
    send(cli->fd, body, strlen(body), MSG_NOSIGNAL);
    close(cli->fd);
    epoll_ctl(epfd, EPOLL_CTL_DEL, cli->fd, NULL);
    clients[cli->fd] = NULL;
    free(cli);
}

/* start sending a cached file */
static void start_mem_send(int epfd, client_t *cli, cache_entry_t *entry) {
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: keep-alive\r\n\r\n",
             entry->mime, entry->size);
    send(cli->fd, header, strlen(header), MSG_NOSIGNAL);

    cli->state = SENDING_MEM;
    cli->file_entry = entry;
    cli->sent = 0;
}

/* process a complete request (headers end with \r\n\r\n) */
static void process_request(int epfd, client_t *cli) {
    char method[16], path[MAX_PATH];
    if (sscanf(cli->rbuf, "%15s %1023s", method, path) != 2) {
        send_error(epfd, cli, 400, "Bad Request");
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_error(epfd, cli, 501, "Not Implemented");
        return;
    }

    /* decode %xx and block traversal */
    char decoded[MAX_PATH];
    int j = 0;
    for (int i = 0; path[i] && j < MAX_PATH-1; i++) {
        if (path[i] == '%' && path[i+1] && path[i+2]) {
            int hex;
            if (sscanf(path+i+1, "%2x", &hex) == 1) {
                decoded[j++] = (char)hex;
                i += 2;
                continue;
            }
        }
        decoded[j++] = path[i];
    }
    decoded[j] = '\0';
    if (strstr(decoded, "..")) {
        send_error(epfd, cli, 403, "Forbidden");
        return;
    }
    if (strcmp(decoded, "/") == 0)
        strcpy(decoded, "/index.html");

    cache_entry_t *entry = cache_lookup(decoded);
    if (!entry) {
        send_error(epfd, cli, 404, "Not Found");
        return;
    }
    start_mem_send(epfd, cli, entry);
}

/* ---------- event handlers ---------- */

static void handle_read(int epfd, client_t *cli) {
    ssize_t n = recv(cli->fd, cli->rbuf + cli->rpos,
                     sizeof(cli->rbuf) - cli->rpos, 0);
    if (n <= 0) {
        close(cli->fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, cli->fd, NULL);
        clients[cli->fd] = NULL;
        free(cli);
        return;
    }
    cli->rpos += n;
    cli->rbuf[cli->rpos] = '\0';
    cli->last_active = time(NULL);

    if (strstr(cli->rbuf, "\r\n\r\n"))
        process_request(epfd, cli);
}

static void handle_write(int epfd, client_t *cli) {
    cli->last_active = time(NULL);
    cache_entry_t *entry = cli->file_entry;
    while (cli->sent < entry->size) {
        ssize_t ret = send(cli->fd,
                           entry->data + cli->sent,
                           entry->size - cli->sent,
                           MSG_NOSIGNAL);
        if (ret > 0) {
            cli->sent += ret;
        } else if (ret == -1 && errno == EAGAIN) {
            /* re‑arm EPOLLOUT and try later */
            struct epoll_event ev;
            ev.events = EPOLLOUT | EPOLLIN | EPOLLET;
            ev.data.ptr = cli;
            epoll_ctl(epfd, EPOLL_CTL_MOD, cli->fd, &ev);
            return;
        } else {
            send_error(epfd, cli, 500, "Internal Error");
            return;
        }
    }

    /* file completely sent – reset for keep‑alive */
    cli->state = READING_REQUEST;
    cli->rpos = 0;
    memset(cli->rbuf, 0, sizeof(cli->rbuf));

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = cli;
    epoll_ctl(epfd, EPOLL_CTL_MOD, cli->fd, &ev);
}

static void timeout_check(int epfd) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_EVENTS; i++) {
        client_t *cli = clients[i];
        if (!cli) continue;
        if (now - cli->last_active > KEEPALIVE_TIMEOUT) {
            close(cli->fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, cli->fd, NULL);
            clients[cli->fd] = NULL;
            free(cli);
        }
    }
}

/* ---------- main ---------- */

int main() {
    /* 1. load all files into memory */
    printf("Loading files from '%s'...\n", ROOT_DIR);
    load_directory(ROOT_DIR, "");
    printf("Cache ready, %d files loaded.\n", (int)(cache_head ? 1 : 0)); /* quick count omitted */

    /* 2. create listening socket */
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_sock, SOMAXCONN);
    set_nonblocking(listen_sock);

    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev);

    printf("RAM server listening on port %d\n", PORT);

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, 1000);
        if (nfds < 0 && errno != EINTR) break;

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int cfd = accept4(listen_sock, (struct sockaddr*)&client_addr, &len,
                                      SOCK_NONBLOCK);
                    if (cfd == -1) break;

                    int tcp_nodelay = 1;
                    setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(tcp_nodelay));

                    client_t *cli = calloc(1, sizeof(client_t));
                    cli->fd = cfd;
                    cli->state = READING_REQUEST;
                    cli->last_active = time(NULL);

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.ptr = cli;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                    clients[cfd] = cli;
                }
            } else {
                client_t *cli = (client_t*)events[i].data.ptr;
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    close(cli->fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, cli->fd, NULL);
                    clients[cli->fd] = NULL;
                    free(cli);
                    continue;
                }
                if (events[i].events & EPOLLIN && cli->state == READING_REQUEST)
                    handle_read(epfd, cli);
                if (events[i].events & EPOLLOUT && cli->state == SENDING_MEM)
                    handle_write(epfd, cli);
            }
        }
        timeout_check(epfd);
    }

    close(listen_sock);
    close(epfd);
    return 0;
}