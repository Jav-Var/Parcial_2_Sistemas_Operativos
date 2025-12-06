#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int cantidad, char *args[]){ //Cantidad de argumentos y arreglo de argumentos

    if (cantidad != 4) {
        printf("Uso correcto: %s <ip_recolector> <puerto> <ip_logica_agente>\n", args[0]);
        return 1;
    }

    //Extraer y parsear argumentos
    char *IP_COLLECTOR = args[1];
    int PORT = atoi(args[2]); //Convertir de string a entero
    char *IP_AGENT = args[3];

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



    while(1){ //Ciclo infinito para lectura de datos cada dos segundos
        FILE *f = fopen("/proc/stat", "r");

        if (!f){
            perror("fopen");
            return 1;
        }

        //Falta calcular los deltas
        char cpu_label[256];
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal;

        if(fscanf(f, "%4s %lu %lu %lu %lu %lu %lu %lu %lu", 
                cpu_label, &user, &nice, &system, &idle, 
                &iowait, &irq, &softirq, &steal) == 9){    
             
            //Pruebas
            printf("Label: %s\n", cpu_label);
            printf("user=%lu nice=%lu system=%lu idle=%lu\n",
                user, nice, system, idle);
        }else{
            fprintf(stderr, "No se pudo leer la linea de cpu\n");
        }

        fclose(f);
        sleep(2);
    }
}