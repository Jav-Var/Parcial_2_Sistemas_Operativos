#include "common.h"
#include "handle_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>

#define BACKLOG     4

#define SHM_KEY 0x7418

int main(int argc, char *argv[]) {
    
    /* --- Recibe port como argumento --- */
    
    if (argc < 2) {
        printf("Uso correcto: %s <puerto>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    /* --- Segmento de memoria compartida --- */
    
    int shmid, semid;
    struct host_info *hosts;
    shmid = shmget(SHM_KEY, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0644);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    hosts = (struct host_info*) shmat(shmid, NULL, 0); 
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    for (int i = 0; i < MAX_HOSTS; i++) {
        hosts[i].active = false;
    } 

    /* --- Crea el semaforo compartido con el visualizador --- */

    semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        if (errno == EEXIST) {
            /* Si ya existe, abrir el existente */
            semid = semget(SEM_KEY, 1, 0666);
            if (semid == -1) {
                perror("semget abrir existente");
                exit(1);
            }
        } else {
            perror("semget create");
            exit(1);
        }
    } 
    // inicializamos el semaforo
    union semun arg;
    arg.val = 1; // semáforo inicial en 1 = disponible 
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        semctl(semid, 0, IPC_RMID);
        exit(1);
    }
    printf("Semáforo creado e inicializado (id=%d, valor=1)\n", semid);

    /* --- Configuracion de socket --- */
    int socket_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    pthread_t threads[MAX_HOSTS];
    
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

        int thread_id = -1;
        for (int i = 0; i < MAX_HOSTS; i++) {
            if (hosts[i].active == false) {
                thread_id = i;
                break;
            }
        }
        if (thread_id == -1) {
            fprintf(stderr, "Error: Maxima cantidad de conexiones");
            close(new_socket);
            continue;
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
        client->data = &hosts[thread_id];
        inet_ntop(AF_INET, &address.sin_addr, client->ip_str, INET_ADDRSTRLEN);
        
        // Create thread for new client
        if (pthread_create(&threads[thread_id], NULL, handle_host, (void*)client) != 0) {
            perror("Thread creation failed");
            free(client);
            close(new_socket);
            continue;
        }
        
        // Detach thread so it cleans up automatically
        pthread_detach(threads[thread_id]);
    }

    return 0;
}