#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>

// Keys de memoria compartida y semaforo
#define SHM_KEY 0x7418 
#define SEM_KEY 0x7419

#define MAX_HOSTS 4

struct host_info {
    char ip[32];
    float cpu_usage;
    float cpu_user;
    float cpu_system;
    float cpu_idle;
    float mem_used_mb;
    float mem_free_mb;
    float swap_total_mb;
    float swap_free_mb;
    uint16_t last_connection_ms; // 0 = Nunca recibido
    uint64_t last_mem_ms;   
    uint64_t last_cpu_ms;   
    bool active; // Se marca como activo si hay un hilo usando el struct
};

static inline uint64_t now_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

// Estructura necesaria para semctl (segun el estandar a veces no viene definida por defecto)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void semaphore_p(int semid);
void semaphore_v(int semid);

#endif