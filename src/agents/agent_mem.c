#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){ //Cantidad de argumentos y arreglo de argumentos

    if (argc != 4) {
        printf("Uso correcto: %s <ip_recolector> <puerto> <ip_logica_agente>\n", argv[0]);
        return 1;
    }

    //Extraer y parsear argumentos
    char *IP_COLLECTOR = argv[1];
    int PORT = atoi(argv[2]); //Convertir de string a entero
    char *IP_AGENT = argv[3];

    //Pruebas de argumentos
    printf("IP Recolector: %s\n", IP_COLLECTOR);
    printf("Puerto: %d\n", PORT);
    printf("IP Logica Agente: %s\n", IP_AGENT);

    struct sockaddr_in server_addr;

    //Creacion del socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); // Inicializar a cero
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    int i = inet_pton(AF_INET, IP_COLLECTOR, &server_addr.sin_addr);

    if(i <= 0){
        perror("inet_pton");
        return 1;
    }

    //Conexion al recolector
    i = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(i < 0){
        perror("connect");
        return 1;
    }

    char buffer[256]; //Buffer para enviar datos al recolector


    //Ciclo infinito para lectura de datos cada dos segundos
    while(1){ 
        FILE *f = fopen("/proc/meminfo", "r");

        if (!f){
            perror("fopen");
            return 1;
        }

        char line[256];
        long mem_total = 0, mem_free = 0, mem_avaliable = 0, swap_total = 0, swap_free = 0; //Variables en KB
        float mem_used_MB; //Usamos float para obtener el valor real en MB


        while(fgets(line, sizeof(line), f)){
            if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
            if (sscanf(line, "MemFree: %ld kB", &mem_free) == 1) continue;
            if (sscanf(line, "MemAvailable: %ld kB", &mem_avaliable) == 1) continue;
            if (sscanf(line, "SwapTotal: %ld kB", &swap_total) == 1) continue;
            if (sscanf(line, "SwapFree: %ld kB", &swap_free) == 1) continue;
        }
        
        //Convertir en MB

        mem_used_MB = (mem_total - mem_avaliable) / 1024.0;
        float mem_free_MB = mem_free / 1024.0;
        float swap_total_MB = swap_total / 1024.0; 
        float swap_free_MB = swap_free / 1024.0; 

        /*Pruebas
        printf("MemFree: %.2f MB\n", mem_free_MB);
        printf("MemUsed: %.2f MB\n", mem_used_MB);
        printf("SwapTotal: %.2f MB\n", swap_total_MB);
        printf("SwapFree: %.2f MB\n", swap_free_MB);
        printf("-----------------------\n");
        */

        //Texto formatado para enviar al recolector
        snprintf(buffer, sizeof(buffer),"MEM;%s;%.2f;%.2f;%.2f;%.2f\n", IP_AGENT, mem_used_MB, mem_free_MB, swap_total_MB, swap_free_MB);


        send(sock, buffer, strlen(buffer), 0);

        fclose(f);
        sleep(2);
    }
}