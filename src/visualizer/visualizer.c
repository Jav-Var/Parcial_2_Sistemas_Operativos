#include "common.h"
#include "table_display.h"    // Función de tabla
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

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