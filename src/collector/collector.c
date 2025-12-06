#include "parser.h"
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>

// Socket constants
#define LISTEN_PORT 9000
#define BACKLOG     16
#define MAX_HOSTS   4
#define MAX_CONN    (MAX_HOSTS * 2) 

// Shared memory constants
#define MAX_HOSTS 4
#define SHM_KEY 0x7418

static inline uint64_t time_now_ms() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)(ts.tv_nsec / 1000000ull);
}

int main() {
    int shmid;
    struct host_info *hosts;
    
    // Shared memory segment
    shmid = shmget(SHM_KEY, sizeof(struct host_info) * MAX_HOSTS, IPC_CREAT | 0644);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    hosts = (struct host_info*)shmat(shmid, NULL, 0); // Attach shared memory to process address space
    if (hosts == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    
    return 0;
}