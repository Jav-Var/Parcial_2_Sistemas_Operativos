#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

int main(int argc, char *argv[]){ // Cantidad de argumentos y arreglo de argumentos
    if (argc != 4) {
        printf("Uso correcto: %s <ip_recolector> <puerto> <ip_logica_agente>\n", argv[0]);
        return 1;
    }

    /* --- Extraer argumentos --- */

    char *IP_COLLECTOR = argv[1];
    int PORT = atoi(argv[2]); // Convertir de string a entero
    char *IP_AGENT = argv[3];

    printf("IP Recolector: %s\n", IP_COLLECTOR);
    printf("Puerto: %d\n", PORT);
    printf("IP Logica Agente: %s\n", IP_AGENT);

    /* --- Creacion del socket --- */
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // Inicializar a cero
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    int r = inet_pton(AF_INET, IP_COLLECTOR, &server_addr.sin_addr);
    if (r == 0) {
        fprintf(stderr, "inet_pton: direccion IP invalida: %s\n", IP_COLLECTOR);
        close(sock);
        return 1;
    } else if (r < 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    /* --- Conexion al colector ---- */

    printf("Conectando al colector...\n");
    r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0){
        perror("connect");
        close(sock);
        return 1;
    }
    printf("Conectado al colector\n");


    /* ---- Ciclo de lectura datos y envio al colector ---- */

    char buffer[512]; // Buffer para enviar datos al recolector

    // Inicializar variables previas de CPU
    unsigned long user_prev =0, nice_prev =0, system_prev =0, idle_prev=0; 
    bool first_read = true;

    while(1){ 

        /* --- Extraccion de datos de CPU --- */

        FILE *f_cpu = fopen("/proc/stat", "r");
        if(!f_cpu){
            perror("fopen");
            fclose(f_cpu);
            return 1;
        }

        char cpu_label[256];
        unsigned long user, nice, system, idle;

        if(fscanf(f_cpu, "%4s %lu %lu %lu %lu", cpu_label, &user, &nice, &system, &idle) != 5){    
            fprintf(stderr, "Error leyendo la linea del cpu \n");
            fclose(f_cpu);
            continue;
        }   

        fclose(f_cpu);

        if(first_read){ // Si es la primera lectura de cpu, continuar y obtener otra lectura
            user_prev = user;
            nice_prev = nice;
            system_prev = system;
            idle_prev = idle;
            first_read = false;

            sleep(2);
            continue;
        }
    
        // Calculo de deltas
        unsigned long delta_user = user - user_prev;
        unsigned long delta_nice = nice - nice_prev;
        unsigned long delta_system = system - system_prev;
        unsigned long delta_idle = idle - idle_prev;
        
        float cpu_total = delta_user + delta_nice + delta_system + delta_idle;
        float cpu_idle = delta_idle;
        float cpu_usage = ((cpu_total - cpu_idle) / cpu_total) * 100.0;

        float user_pct = (delta_user * 100.0) / cpu_total;
        float system_pct = (delta_system * 100.0) / cpu_total;
        float idle_pct = (delta_idle * 100.0) / cpu_total;

        // Actualizar valores previos
        user_prev = user;
        nice_prev = nice;
        system_prev = system;
        idle_prev = idle;

        /* --- Extraccion de datos de memoria --- */
        
        FILE *f_mem = fopen("/proc/meminfo", "r");
        if (!f_mem){
            perror("fopen");
            return 1;
        }

        // Datos de memoria en KB
        char mem_label[256];
        long mem_total = 0, mem_free = 0, mem_avaliable = 0, swap_total = 0, swap_free = 0; // Variables en KB
        float mem_used_MB; // Usamos float para obtener el valor real en MB

        while(fgets(mem_label, sizeof(mem_label), f_mem)){
            if (sscanf(mem_label, "MemTotal: %ld kB", &mem_total) == 1) continue;
            if (sscanf(mem_label, "MemFree: %ld kB", &mem_free) == 1) continue;
            if (sscanf(mem_label, "MemAvailable: %ld kB", &mem_avaliable) == 1) continue;
            if (sscanf(mem_label, "SwapTotal: %ld kB", &swap_total) == 1) continue;
            if (sscanf(mem_label, "SwapFree: %ld kB", &swap_free) == 1) continue;
        }

        fclose(f_mem);

        // Convertir a MB
        mem_used_MB = (mem_total - mem_avaliable) / 1024.0;
        float mem_free_MB = mem_free / 1024.0;
        float swap_total_MB = swap_total / 1024.0; 
        float swap_free_MB = swap_free / 1024.0; 

        /* --- Envio de datos al colector --- */

        // Formato <ip_logica_agente>;<mem_used_MB>;<MemFree_MB>;<SwapTotal_MB>;<SwapFree_MB>;<CPU_usage>;<user_pct>;<system_pct>;<idle_pct>\n
        snprintf(
            buffer, sizeof(buffer), "%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",
            IP_AGENT, mem_used_MB, mem_free_MB, swap_total_MB, swap_free_MB, cpu_usage, user_pct, system_pct, idle_pct
        );
        
        send(sock, buffer, strlen(buffer), 0);

        sleep(2);
    }
}