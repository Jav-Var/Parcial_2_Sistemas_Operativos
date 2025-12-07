#include "../collector/parser.h"       // Estructura compartida
#include "table_display.h"    // Función de tabla
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

// Definimos keys aquí hardcoded (estilo "raw")
#define SHM_KEY 0x7418 
#define SEM_KEY 0x7419  // Misma key que usará el collector

// Estructura necesaria para semctl (a veces no viene definida por defecto)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// --- Funciones de Semáforo (Locales) ---

void semaphore_p(int semid) {
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1; // Restar 1 (Bloquear/Wait)
    sb.sem_flg = 0;
    
    if (semop(semid, &sb, 1) == -1) {
        perror("Error bloqueando semáforo");
        exit(1);
    }
}

void semaphore_v(int semid) {
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = 1;  // Sumar 1 (Liberar/Signal)
    sb.sem_flg = 0;
    
    if (semop(semid, &sb, 1) == -1) {
        perror("Error liberando semáforo");
        exit(1);
    }
}

// --- Main ---

int main() {
    int shmid, semid;
    struct host_info *hosts;
    
    printf("Iniciando visualizador...\n");

    // 1. Conectar a Memoria Compartida
    shmid = shmget(SHM_KEY, 0, 0666);
    if (shmid == -1) {
        perror("Fallo shmget (¿Está corriendo el collector?)");
        exit(1);
    }
    
    hosts = (struct host_info*) shmat(shmid, NULL, 0);
    if (hosts == (void*)-1) {
        perror("Fallo shmat");
        exit(1);
    }

    // 2. Conectar al Semáforo (Ya debe haberlo creado el collector)
    semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("Fallo semget (El collector debe iniciar primero para crear el semáforo)");
        exit(1);
    }

    printf("Conexión exitosa. Mostrando datos...\n");

    // 3. Loop infinito
    while (1) {
        // Entrar a Sección Crítica
        semaphore_p(semid);
        
        // Leer y Mostrar
        update_table(hosts);
        
        // Salir de Sección Crítica
        semaphore_v(semid);

        // Refrescar cada 1 segundo
        sleep(1);
    }

    // Código inalcanzable pero formalmente correcto
    shmdt(hosts);
    return 0;
}