#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

/* Example updated struct */
struct host_info {
    char     ip[32];
    double   cpu_usage;
    double   cpu_user;
    double   cpu_system;
    double   cpu_idle;
    uint64_t mem_used_mb;
    uint64_t mem_free_mb;
    uint64_t swap_total_mb;
    uint64_t swap_free_mb;
    uint64_t last_mem_ms;   /* 0 = never received */
    uint64_t last_cpu_ms;   /* 0 = never received */
};


