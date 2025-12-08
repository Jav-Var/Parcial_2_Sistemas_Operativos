#include <stdio.h>
#include <errno.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>

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
