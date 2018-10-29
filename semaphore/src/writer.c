
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "common.h"

int main(int argc, char const *argv[])
{
    // Get pagesize
    long bufsize = sysconf(_SC_PAGESIZE);
    printf("bufsz  = %ld\n", bufsize);

    key_t key = 0;
    GET_SYSV_KEY(key, KEY_NAME, KEY_PROJ);
    printf("key    = %d\n", key);

    int sem_set_id = 0;
    SEMGET(sem_set_id, key, N_SEMS, IPC_CREAT | 0666);
    printf("set id = %d\n", sem_set_id);

    int shm_id = 0;
    SHMGET(shm_id, key, bufsize, IPC_CREAT | 0666);
    printf("shm_id = %d\n", shm_id);

    char* buffer = NULL;
    SHMAT(char*, buffer, shm_id, NULL, 0);



    getchar();
    // Actually, action



    // Detach memory
    errno = 0;
    if ( shmdt(buffer) != 0 )
    {
        perror("shmdt");
        return EXIT_FAILURE;
    }

    // Delete semaphores
    if ( semctl(sem_set_id, 0, IPC_RMID) != 0 && errno != EINVAL )
    {
        perror("semctl");
        return EXIT_FAILURE;
    }

    // Delete shared memory
    if ( shmctl(shm_id, IPC_RMID, NULL)  != 0 && errno != EINVAL )
    {
        perror("shmctl");
        return EXIT_FAILURE;
    }

    return 0;
}


