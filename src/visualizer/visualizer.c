#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAX_HOSTS 4
#define SHM_KEY 0x7418  // Must match writer's key

int main() {
    int shmid;
    struct host_info *hosts;
    
    // Get existing shared memory
    shmid = shmget(SHM_KEY, 0, 0666);
    if (shmid == -1) {
        perror("shmget failed - is collector running?");
        exit(1);
    }
    
    hosts = (struct host_info*) shmat(shmid, NULL, 0);
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    return 0;
}