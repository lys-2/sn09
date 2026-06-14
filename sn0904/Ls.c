#if defined(__linux__)

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 1. Create a UDP Socket (SOCK_DGRAM)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Configure server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all network interfaces
    server_addr.sin_port = htons(PORT);       // Host-to-network short byte order

    // 3. Bind the socket to the port
    if (bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("UDP Server listening on port %d...\n", PORT);

    // 4. Main communication loop
    while (1) {
        // Block until data arrives, storing sender info in client_addr
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
            (struct sockaddr*)&client_addr, &addr_len);
        if (bytes_received < 0) {
            perror("Receive failed");
            continue;
        }

        // Null-terminate the string safely
        buffer[bytes_received] = '\0';
        printf("Received: %s", buffer);

        // Echo the same data back to the client
        sendto(sockfd, buffer, bytes_received, 0,
            (const struct sockaddr*)&client_addr, addr_len);
    }

    // 5. Clean up (unreachable in infinite loop, included for completeness)
    close(sockfd);
    return 0;
}

#endif
