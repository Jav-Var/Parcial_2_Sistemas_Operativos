
#ifndef HANDLE_HOST_H

#define HANDLE_HOST_H
#define _GNU_SOURCE

#include<sys/socket.h>
#include<arpa/inet.h>
#include<time.h>
#include<stdbool.h>

struct connection_info {
    int socket;
    struct sockaddr_in address;
    char ip_str[INET_ADDRSTRLEN];
    struct host_info *data;
    int semid; // Semaforo para sincronizar escritura con el visualizador
};

void* handle_host(void* arg);

#endif