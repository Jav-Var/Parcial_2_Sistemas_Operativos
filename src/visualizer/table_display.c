#define _GNU_SOURCE
#include "table_display.h"
#include "common.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>

void update_table(struct host_info *hosts) {
    
    /* --- Limpia la pantalla e imprime la cabecera de la tabla --- */

    printf("\033[H\033[J"); 

    printf("=== MONITOR DE SISTEMA ===\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("| %-15s | %-6s | %-6s | %-8s | %-8s | %-8s | %-6s |\n", 
           "IP", "CPU %", "Mem %", "Mem Used", "Mem Free", "Swap Use", "Status");
    printf("|-----------------|--------|--------|----------|----------|----------|--------|\n");

    /* -- Imprime los datos de cada host --- */

    uint64_t now = now_ms();

    for (int i = 0; i < MAX_HOSTS; i++) {
        if (hosts[i].active && hosts[i].ip[0] != '\0') {

            // Calculamos la inactividad
            uint64_t diff = (now >= hosts[i].last_connection_ms) ? (now - hosts[i].last_connection_ms) : 0;
            
            // 2. Decidimos qué imprimir basado en el tiempo
            if (diff > 5000) {
                // --- CASO DESCONECTADO (> 5 seg) ---
                // Muestra la IP, pero el resto son guiones. Estado "DOWN".
                printf("| %-15s |   ---  |   ---  |    ---   |    ---   |    ---   | %-6s |\n",
                       hosts[i].ip, "DOWN");
            } 
            else {
                // --- CASO CONECTADO (Funcionamiento normal) ---
                
                // Calculo de porcentaje de memoria
                double mem_pct = 0.0;
                double total_mem = hosts[i].mem_used_mb + hosts[i].mem_free_mb;
                
                if (total_mem > 0) {
                    mem_pct = (hosts[i].mem_used_mb / total_mem) * 100.0;
                }

                // Imprimimos los datos reales
                printf("| %-15s | %5.1f%% | %5.1f%% | %5.0f MB | %5.0f MB | %5.0f MB | %-6s |\n",
                       hosts[i].ip,
                       hosts[i].cpu_usage,
                       mem_pct,
                       hosts[i].mem_used_mb,
                       hosts[i].mem_free_mb,
                       (hosts[i].swap_total_mb - hosts[i].swap_free_mb),
                       "OK");
            }

        } else {
            // --- CASO SLOT VACÍO ---
            printf("| %-15s |   --   |   --   |    --    |    --    |    --    |   --   |\n", "---");
        }
    }
    printf("-------------------------------------------------------------------------------\n");
    printf("Presione Ctrl+C para salir.\n");
}