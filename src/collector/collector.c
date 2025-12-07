#include "parser.h"
#include "handle_host.h"
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

// Socket constants
#define LISTEN_PORT 9000
#define BACKLOG     16
#define MAX_HOSTS   4
#define MAX_CONN    (MAX_HOSTS * 2) 

// Shared memory constants
#define MAX_HOSTS 4
#define SHM_KEY 0x7418

static inline uint64_t time_now_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

struct client_info {
    int socket;
    struct sockaddr_in address;
    char ip_str[INET_ADDRSTRLEN];
    int thread_id;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso correcto: ./collector <puerto>");
        exit(1);
    }

    int port = atoi(argv[2]);
    int shmid;
    struct host_info *hosts;
    
    /* --- Segmento de memoria compartida con el visualizador --- */
    shmid = shmget(SHM_KEY, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0644);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    hosts = (struct host_info*)shmat(shmid, NULL, 0); // Attach shared memory to process address space
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    /* --- Configuracion de socket --- */
    int socket_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t threads[MAX_CONN];
    int thread_count = 0;
    
    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Bind socket
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(socket_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    

    /* --- Server loop --- */
    while (1) {
        // Accept new connection
        if ((new_socket = accept(socket_fd, (struct sockaddr*)&address, 
                                (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Create client info structure
        struct client_info* client = malloc(sizeof(struct client_info));
        if (!client) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        
        client->socket = new_socket;
        client->address = address;
        inet_ntop(AF_INET, &address.sin_addr, client->ip_str, INET_ADDRSTRLEN);
        
        // Create thread for new client
        if (pthread_create(&threads[thread_count % MAX_CONN], NULL, 
                          handle_client, (void*)client) != 0) {
            perror("Thread creation failed");
            free(client);
            close(new_socket);
            continue;
        }
        
        // Detach thread so it cleans up automatically
        pthread_detach(threads[thread_count % MAX_CONN]);
        thread_count++;
    }

    return 0;
}