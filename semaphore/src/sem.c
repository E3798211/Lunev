#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

// #define KEY_NAME "/"
// #define KEY_PROJ 777

// #define N_SEMS   7

#include "common.h"


int main(int argc, char const *argv[])
{
    // Get key
    key_t key = 0;
    errno = 0;
    if ( (key = ftok(KEY_NAME, KEY_PROJ)) == -1)
    {
        perror("ftok");
        return EXIT_FAILURE;
    }
    printf("key    = %d\n", key);

    // Get semaphores set
    errno = 0;
    int sem_set_id = semget(key, N_SEMS, IPC_CREAT | 0666);
    if (sem_set_id == -1 && errno != EEXIST)
    {
        perror("semget");
        return EXIT_FAILURE;
    }
    printf("set id = %d\n", sem_set_id);

    // Get pagesize
    long bufsize = sysconf(_SC_PAGESIZE);
    printf("bufsz  = %ld\n", bufsize);

    // Get shared memory
    errno = 0;
    int shm_id = shmget(key, bufsize, IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("shmget");
        return EXIT_FAILURE;
    }

    // Attach memory
    errno = 0;
    char* buffer = (char*)shmat(shm_id, NULL, 0);
    if (buffer ==  (char*)-1)
    {
        perror("shmat");
        return EXIT_FAILURE;
    }

// =======================================================

//    printf("here\n");
    for(int i = 0; i < N_SEMS; i++)
        printf("%d ", semctl(sem_set_id, i, GETVAL));
    printf("\n");

//    return 0;

    // Actually, action

    #define N_SEMS 2

    struct sembuf sem_operations[N_SEMS];
    sem_operations[0].sem_num = 0;
    sem_operations[0].sem_op  = -5;
    sem_operations[0].sem_flg = IPC_NOWAIT;
    
    sem_operations[1].sem_num = 0;
    sem_operations[1].sem_op  = -1;  
    sem_operations[1].sem_flg = 0;

    struct sembuf op;
    op.sem_num = 0;
    op.sem_op  = 20;
    op.sem_flg = 0;

    printf("before operation\n");
    for(int i = 0; i < N_SEMS; i++)
        printf("%d ", semctl(sem_set_id, i, GETVAL));
    printf("\n");

    if ( semop(sem_set_id, sem_operations, 2) == -1 )
    {
        perror("semop");
        return EXIT_FAILURE;
    }

/*
    if ( semop(sem_set_id, &op, 1) == -1 )
    {
        perror("semop");
        return EXIT_FAILURE;
    }
 */

    printf("after operation\n");
    for(int i = 0; i < N_SEMS; i++)
        printf("%d ", semctl(sem_set_id, i, GETVAL));
    printf("\n");


    getchar();


    // Detach memory
    errno = 0;
    if ( shmdt(buffer) != 0 )
    {
        perror("shmdt");
        return EXIT_FAILURE;
    }
/*
    // Delete semaphores
    if ( semctl(sem_set_id, 0, IPC_RMID) != 0 && errno != EINVAL )
    {
        perror("semctl");
        return EXIT_FAILURE;
    }
 */

    // Delete shared memory
    if ( shmctl(shm_id, IPC_RMID, NULL)  != 0 && errno != EINVAL )
    {
        perror("shmctl");
        return EXIT_FAILURE;
    }

    return 0;
}


