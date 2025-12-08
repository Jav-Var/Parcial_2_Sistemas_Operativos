#include "common.h"
#include "handle_host.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void* handle_host(void* arg) {
    struct connection_info* client = (struct connection_info*)arg;
    if (client == NULL) {
        return NULL;
    }

    int sock = client->socket;
    struct host_info* h = client->data;
    if (h == NULL) {
        close(sock);
        free(client);
        return NULL;
    }

    /* read loop: accumulate until newline, handle partial reads */
    char buf[1024];
    size_t pos = 0;
    ssize_t n;

    /* mark inactive until we successfully parse first message */
    h->active = false;
    int semid = client->semid;

    while ((n = recv(sock, buf + pos, sizeof(buf) - 1 - pos, 0)) > 0) {
        pos += (size_t)n;
        buf[pos] = '\0';

        /* process every full line (terminated by '\n') in the buffer */
        char *line_start = buf;
        char *nl;
        while ((nl = strchr(line_start, '\n')) != NULL) {
            *nl = '\0';
            /* strip optional '\r' at end (handle CRLF) */
            if (nl > line_start && *(nl - 1) == '\r') *(nl - 1) = '\0';

            /* expected message:
               <ip>;<mem_used_MB>;<MemFree_MB>;<SwapTotal_MB>;<SwapFree_MB>;<CPU_usage>;<user_pct>;<system_pct>;<idle_pct>
            */
            char ip_str[32];
            float mem_used, mem_free, swap_total, swap_free;
            float cpu_usage, user_pct, system_pct, idle_pct;

            int parsed = sscanf(
                line_start, "%31[^;];%f;%f;%f;%f;%f;%f;%f;%f",
                ip_str, &mem_used, &mem_free, &swap_total, &swap_free, &cpu_usage, &user_pct, &system_pct, &idle_pct
            );

            if (parsed == 9) {
                /* update host_info (best-effort; no external locking is assumed) */
                /* copy IP - ensure null termination */
                semaphore_p(semid);
                
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

                printf(
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
                    h->ip,
                    h->mem_used_mb,
                    h->mem_free_mb,
                    h->swap_total_mb,
                    h->swap_free_mb,
                    h->cpu_usage,
                    h->cpu_user,
                    h->cpu_system,
                    h->cpu_idle
                );

                /* use the same timestamp for cpu and mem updates */
                uint64_t now = now_ms();
                h->last_mem_ms = now;
                h->last_cpu_ms = now;

                h->active = true;

                semaphore_v(semid);

            } else {
                /* parsing failed; ignore this line but keep connection open */
                fprintf(stderr, "collector: failed to parse line from %s: '%s'\n",
                        client->ip_str[0] ? client->ip_str : "unknown", line_start);
            }

            /* advance to the next line */
            line_start = nl + 1;
        }

        /* move any remaining partial data to front of buffer */
        size_t remain = (size_t)(buf + pos - line_start);
        if (remain > 0 && line_start != buf) {
            memmove(buf, line_start, remain);
        }
        pos = remain;

        /* if buffer full with no newline, drop it to avoid overflow */
        if (pos == sizeof(buf) - 1) {
            pos = 0;
        }
    }

    if (n == 0) {
        /* orderly shutdown by peer */
        h->active = false;
    } else if (n < 0) {
        /* recv error */
        perror("recv");
        h->active = false;
    }

    close(sock);
    free(client);
    return NULL;
}
