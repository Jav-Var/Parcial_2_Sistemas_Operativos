#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> // mover time_now_ms

#ifndef PARSER_H

#define PARSER_H

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

void parse_mem_data(char *line, struct host_info *info_ptr);
void parse_cpu_data(char *line, struct host_info *info_ptr);




#endif