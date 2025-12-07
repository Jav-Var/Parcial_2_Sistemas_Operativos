#include "table_display.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// Función auxiliar local para calcular tiempo actual
static uint64_t current_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

void update_table(struct host_info *hosts) {
    // Limpiar pantalla
    printf("\033[H\033[J"); 

    printf("=== MONITOR DE SISTEMA ===\n");
    printf("--------------------------------------------------------------------------------------\n");
    printf("| %-15s | %-6s | %-6s | %-8s | %-8s | %-8s | %-6s |\n", 
           "IP", "CPU %", "Mem %", "Mem Used", "Mem Free", "Swap Use", "Status");
    printf("|-----------------|--------|--------|----------|----------|----------|--------|\n");

    uint64_t now = current_timestamp();

    for (int i = 0; i < MAX_HOSTS; i++) {
        if (hosts[i].active && hosts[i].ip[0] != '\0') {
            
            // Calculo de porcentaje de memoria
            double mem_pct = 0.0;
            // Sumamos como double para evitar problemas de tipos
            double total_mem = hosts[i].mem_used_mb + hosts[i].mem_free_mb;
            
            if (total_mem > 0) {
                mem_pct = (hosts[i].mem_used_mb / total_mem) * 100.0;
            }

            // Calculo de inactividad
            uint64_t diff = (now >= hosts[i].last_mem_ms) ? (now - hosts[i].last_mem_ms) : 0;
            
            // CORRECCIÓN 1: Aumentamos el buffer a 32 para evitar overflow
            char status[32] = "OK"; 
            
            if (diff > 5000) {
                // CORRECCIÓN 1: Usamos %lu para el tiempo (que sí es entero)
                sprintf(status, "LAG (%lus)", diff/1000);
            }

            // CORRECCIÓN 2: Usamos %5.0f para la memoria. 
            // El .0f le dice que imprima el double sin decimales (parece entero).
            printf("| %-15s | %5.1f%% | %5.1f%% | %5.0f MB | %5.0f MB | %5.0f MB | %-6s |\n",
                   hosts[i].ip,
                   hosts[i].cpu_usage,
                   mem_pct,
                   hosts[i].mem_used_mb,
                   hosts[i].mem_free_mb,
                   (hosts[i].swap_total_mb - hosts[i].swap_free_mb),
                   status);
        } else {
            printf("| %-15s |   --   |   --   |    --    |    --    |    --    |   --   |\n", "---");
        }
    }
    printf("--------------------------------------------------------------------------------------\n");
    printf("Presione Ctrl+C para salir.\n");
}