#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>

#define SHM_KEY 0x7418 
#define SEM_KEY 0x7419  // Misma key que usará el collector
#define MAX_HOSTS 4

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

// Estructura necesaria para semctl (a veces no viene definida por defecto)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void semaphore_p(int semid);
void semaphore_v(int semid);

#endif