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
};

struct host_info {
    char     ip[32];
    float   cpu_usage;
    float   cpu_user;
    float   cpu_system;
    float   cpu_idle;
    float    mem_used_mb;
    float mem_free_mb;
    float swap_total_mb;
    float swap_free_mb;
    uint64_t last_mem_ms;   // 0 = never received
    uint64_t last_cpu_ms;   // 0 = never received
    bool     active;
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