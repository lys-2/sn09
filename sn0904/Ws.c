#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define PORT        12345
#define BUFFER_SIZE 512

static SOCKET g_udpSocket = 0;
static int    g_udpInitialised = 0;
struct sockaddr_in serverAddr;

/* Initialise UDP socket (non‑blocking) */
int udp_send(char* m) {
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &clientAddr.sin_addr);
    sendto(g_udpSocket, m, (int)strlen(m), 0,
        (struct sockaddr*)&clientAddr, clientAddrLen);
}

int udp_init() {
    WSADATA wsaData;
    u_long mode = 1;  /* non‑blocking mode */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 0;
    }

    g_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (g_udpSocket == INVALID_SOCKET) {
        printf("socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    /* Set non‑blocking */
    if (ioctlsocket(g_udpSocket, FIONBIO, &mode) != 0) {
        printf("ioctlsocket failed: %d\n", WSAGetLastError());
        closesocket(g_udpSocket);
        WSACleanup();
        return 0;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(g_udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(g_udpSocket);
        WSACleanup();
        return 0;
    }

    g_udpInitialised = 1;
    printf(" listening on port %d\n", PORT);
    return 1;
}

/* Poll for one incoming UDP packet (non‑blocking) */
void udp_poll() {
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    int recvLen;

    if (!g_udpInitialised) return;

    /* Try to receive (non‑blocking) */
    recvLen = recvfrom(g_udpSocket, buffer, BUFFER_SIZE - 1, 0,
        (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (recvLen == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            printf("recvfrom error: %d\n", err);
        }
        return;   /* no data or real error */
    }

    buffer[recvLen] = '\0';

    /* Print client message */
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    printf("[%s]: %s\n", clientIP, buffer);

    /* Send a reply back */
/*    char reply[BUFFER_SIZE];
    snprintf(reply, sizeof(reply), "Server received: %s", buffer);*/
/*    sendto(g_udpSocket, reply, (int)strlen(reply), 0,
        (struct sockaddr*)&clientAddr, clientAddrLen);*/
}

/* Cleanup */
void udp_cleanup() {
    if (g_udpSocket != INVALID_SOCKET) {
        closesocket(g_udpSocket);
        g_udpSocket = INVALID_SOCKET;
    }
    WSACleanup();
    g_udpInitialised = 0;
}