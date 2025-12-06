#include "parser.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void parse_mem_data(char *line, struct host_info *info_ptr) {
    // Formato: MEM;<ip_logica_agente>;<mem_used_MB>;<MemFree_MB>;<SwapTotal_MB>;<SwapFree_MB>\n
    char *token;
    char *saveptr = NULL;
    
    // Skip "MEM;"
    token = strtok_r(line, ";", &saveptr);
    if (token == NULL) return;
    
    // IP (verifica que cabe en 32 bytes incluyendo el terminador nulo)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) {
        strncpy(info_ptr->ip, token, sizeof(info_ptr->ip) - 1);
        info_ptr->ip[sizeof(info_ptr->ip) - 1] = '\0';
    }
    
    // mem used
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->mem_used_mb = strtoull(token, NULL, 10);
    
    // mem free
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->mem_free_mb = strtoull(token, NULL, 10);
    
    // swap
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->swap_total_mb = strtoull(token, NULL, 10);
    
    // free swap
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->swap_free_mb = strtoull(token, NULL, 10);
}

void parse_cpu_data(char *line, struct host_info *info_ptr) {
    // Formato: CPU;<ip_logica_agente>;<CPU_usage>;<user_pct>;<system_pct>;<idle_pct>\n
    char *token;
    char *saveptr = NULL;
    
    // Skip "CPU;"
    token = strtok_r(line, ";", &saveptr);
    if (token == NULL) return;
    
    // IP (verifica que cabe en 32 bytes incluyendo el terminador nulo)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) {
        strncpy(info_ptr->ip, token, sizeof(info_ptr->ip) - 1);
        info_ptr->ip[sizeof(info_ptr->ip) - 1] = '\0';
    }
    
    // CPU usage percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_usage = atof(token);
    
    // user CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_user = atof(token);
    
    // system CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_system = atof(token);
    
    // idle CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_idle = atof(token);
}