#include "parser.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static uint64_t now_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return 0;
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

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

    // mem used (float)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->mem_used_mb = strtof(token, NULL);

    // mem free (float)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->mem_free_mb = strtof(token, NULL);

    // swap total (float)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->swap_total_mb = strtof(token, NULL);

    // swap free (float)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->swap_free_mb = strtof(token, NULL);

    /* marca tiempo de recepción y activa el host */
    info_ptr->last_mem_ms = now_ms();
    info_ptr->active = true;
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

    // CPU usage percentage (float)
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_usage = strtof(token, NULL);

    // user CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_user = strtof(token, NULL);

    // system CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_system = strtof(token, NULL);

    // idle CPU percentage
    token = strtok_r(NULL, ";", &saveptr);
    if (token) info_ptr->cpu_idle = strtof(token, NULL);

    /* marca tiempo de recepción y activa el host */
    info_ptr->last_cpu_ms = now_ms();
    info_ptr->active = true;
}
