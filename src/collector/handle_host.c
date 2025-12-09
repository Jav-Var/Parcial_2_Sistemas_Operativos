#include "common.h"
#include "handle_host.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void* handle_host(void* arg) {
    
    /* --- Argumentos de la funcion --- */
    
    struct connection_info* client = (struct connection_info*)arg;
    if (client == NULL) {
        return NULL;
    }

    int semid = client->semid; // Semaforo para sincronizar la seccion critica
    int sock = client->socket; // Socket
    struct host_info* h = client->data; // Estructura para guardar los datos
    if (h == NULL) {
        close(sock);
        free(client);
        return NULL;
    }

    /* --- Bucle de lectura, acumula en el buffer hasta que lee una linea completa --- */
    
    char buf[1024]; // Buffer para recibir datos del agente
    size_t pos = 0; // Posicion del buffer en la que se escriben datos recibidos

    ssize_t n; // Cantidad de bytes recibidos
    while ((n = recv(sock, buf + pos, sizeof(buf) - 1 - pos, 0)) > 0) {
        pos += (size_t) n;
        buf[pos] = '\0';

        /* --- Procesa la linea --- */

        char *line_start = buf;
        char *line_end;
        while ((line_end = strchr(line_start, '\n')) != NULL) {  // Procesa las lineas entrantes si contienen '\n'
            *line_end = '\0'; // Cambia '\n' por '\0'

            // Formato: <ip>;<mem_used_MB>;<MemFree_MB>;<SwapTotal_MB>;<SwapFree_MB>;<CPU_usage>;<user_pct>;<system_pct>;<idle_pct>
            char ip_str[32];
            float mem_used, mem_free, swap_total, swap_free;
            float cpu_usage, user_pct, system_pct, idle_pct;

            int parsed = sscanf( // Lee todos los datos de la linea
                line_start, "%31[^;];%f;%f;%f;%f;%f;%f;%f;%f",
                ip_str, &mem_used, &mem_free, &swap_total, &swap_free, &cpu_usage, &user_pct, &system_pct, &idle_pct
            );

            if (parsed == 9) { // Si se parsearon los 9 argumentos correctamente, actualizar datos

                semaphore_p(semid); // Inicio seccion critica

                // Actualiza todos los datos en el struct
                strncpy(h->ip, ip_str, sizeof(h->ip) - 1);
                h->ip[sizeof(h->ip) - 1] = '\0';
                h->mem_used_mb  = mem_used;
                h->mem_free_mb  = mem_free;
                h->swap_total_mb = swap_total;
                h->swap_free_mb = swap_free;
                h->cpu_usage  = cpu_usage;
                h->cpu_user   = user_pct;
                h->cpu_system = system_pct;
                h->cpu_idle   = idle_pct;

                // Muestra los datos recibidos (Para pruebas)
                printf(
                    "Recibido:\n"
                    "ip = %s\n"
                    "mem_used_mb   = %.2f MB\n"
                    "mem_free_mb   = %.2f MB\n"
                    "swap_total_mb = %.2f MB\n"
                    "swap_free_mb  = %.2f MB\n"
                    "cpu_usage     = %.2f%%\n"
                    "cpu_user      = %.2f%%\n"
                    "cpu_system    = %.2f%%\n"
                    "cpu_idle      = %.2f%%\n"
                    "---------------------\n",
                    h->ip, h->mem_used_mb, h->mem_free_mb, h->swap_total_mb, h->swap_free_mb,
                    h->cpu_usage, h->cpu_user, h->cpu_system, h->cpu_idle
                );

                // Actualiza el tiempo de la ultima conexion
                uint64_t now = now_ms();
                h->last_connection_ms = now;
                h->active = true;

                semaphore_v(semid); // Fin de la seccion critica

            } else {
                // Error al parsear la linea, ignora la linea
                fprintf(stderr, "collector: failed to parse line from %s: '%s'\n", client->ip_str[0] ? client->ip_str : "unknown", line_start);
            }

            // Avanza a la nueva linea
            line_start = line_end + 1;
        }

        // Mueve la parte restante al inicio del buffer
        size_t remain = (size_t)(buf + pos - line_start);
        if (remain > 0 && line_start != buf) {
            memmove(buf, line_start, remain);
        }
        pos = remain;

        // Si el buffer se llena y no hay newline, ignora la linea para evitar overflow 
        if (pos == sizeof(buf) - 1) {
            pos = 0;
        }
    }

    /* --- Host desconectado ---- */

    if (n == 0) {
        /* orderly shutdown by peer */
        h->active = false;
    } else if (n < 0) { // recv error
        perror("recv");
        h->active = false;
    }

    close(sock);
    free(client);
    return NULL;
}
