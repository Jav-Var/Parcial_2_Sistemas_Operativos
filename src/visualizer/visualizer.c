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

    printf("Iniciando visualizador...\n");

    /* -- Obtiene el segmento de memoria compartida creado por el colector -- */

    int shmid, semid;
    struct host_info *hosts;

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

    /* -- Obtiene el semaforo creado por el colector -- */

    semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("Fallo semget (El collector debe iniciar primero para crear el semáforo)");
        exit(1);
    }

    /* --- Inicia el bucle para mostrar los datos --- */

    printf("Mostrando datos...\n");

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

    shmdt(hosts);
    return 0;
}