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

static inline uint64_t now_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

void* handle_host(void* arg);

#endif