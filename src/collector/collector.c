#include "handle_host.h"
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BACKLOG     4
#define MAX_HOSTS   4
#define MAX_CONN    (MAX_HOSTS * 2) 

#define SHM_KEY 0x7418

int main(int argc, char *argv[]) {
    /* --- Recibe port como argumento --- */
    if (argc < 2) {
        printf("Uso correcto: %s <puerto>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int shmid;
    struct host_info *hosts;
    
    /* --- Segmento de memoria compartida con el visualizador --- */
    shmid = shmget(SHM_KEY, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0644);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    hosts = (struct host_info*)shmat(shmid, NULL, 0); // incluye la memoria compartida al process address space
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    /* --- Configuracion de socket --- */
    int socket_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    pthread_t threads[MAX_CONN];
    int thread_count = 0;
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Crea el socket
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Opciones de socket para reusar la direccion
    int opt = 1; 
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Configuracion de address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Bind socket
    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Escucha conexiones entrantes
    if (listen(socket_fd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    

    /* --- Server loop --- */
    while (1) {
        // Accept new connection
        if ((new_socket = accept(socket_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        if (thread_count > MAX_CONN) {
            fprintf(stderr, "Error: Maxima cantidad de conexiones");
        }
        
        // Create client info structure
        struct connection_info* client = malloc(sizeof(struct connection_info));
        if (client == NULL) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        
        client->socket = new_socket;
        client->address = address;
        client->data = &hosts[thread_count];
        inet_ntop(AF_INET, &address.sin_addr, client->ip_str, INET_ADDRSTRLEN);
        
        // Create thread for new client
        if (pthread_create(&threads[thread_count], NULL, handle_host, (void*)client) != 0) {
            perror("Thread creation failed");
            free(client);
            close(new_socket);
            continue;
        }
        
        // Detach thread so it cleans up automatically
        pthread_detach(threads[thread_count]);
        thread_count++;
    }

    return 0;
}