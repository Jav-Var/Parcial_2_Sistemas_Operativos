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

int main(int argc, char *argv[]) {
    
    /* --- Recibe port como argumento --- */
    
    if (argc < 2) {
        printf("Uso correcto: %s <puerto>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    /* --- Segmento de memoria compartida --- */
    
    int shmid;
    struct host_info *hosts;
    shmid = shmget(SHM_KEY, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0644);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Array compartido que contiene structs con los datos de los hosts conectados
    hosts = (struct host_info*) shmat(shmid, NULL, 0); 
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    // Marca todos como inactivos
    for (int i = 0; i < MAX_HOSTS; i++) {
        hosts[i].active = false;
    } 

    /* --- Crea el semaforo compartido con el visualizador --- */

    int semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        if (errno == EEXIST) {
            // Si ya existe, abrir el existente
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

    /* --- Creacion del socket --- */

    int socket_fd;
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

    int new_socket;
    while (1) {
        // Acepta la nueva conexion en un nuevo socket
        if ((new_socket = accept(socket_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Busca un slot inactivo
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
        
        // Crea un struct con los datos de la conexion para pasarlos al hilo
        struct connection_info* client = malloc(sizeof(struct connection_info));
        if (client == NULL) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        
        // Escribe los datos de la conexion
        client->socket = new_socket;
        client->address = address;
        client->data = &hosts[thread_id]; // Struct para almacenar los datos de memoria y cpu
        inet_ntop(AF_INET, &address.sin_addr, client->ip_str, INET_ADDRSTRLEN);
        
        // Crea un nuevo hilo para el host
        if (pthread_create(&threads[thread_id], NULL, handle_host, (void*)client) != 0) {
            perror("Thread creation failed");
            free(client);
            close(new_socket);
            continue;
        }

        client->data->active = true;

        // Para que se limpie automáticamente el hilo
        pthread_detach(threads[thread_id]);
    }

    return 0;
}