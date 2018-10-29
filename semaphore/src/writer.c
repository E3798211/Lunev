
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "common.h"

// =========================================================

/*
    Performs the main activity
 */
int WriterAction(int sem_id, char* buffer, long bufsize);

// =========================================================

int main(int argc, char const *argv[])
{
    // Get pagesize
    long bufsize = sysconf(_SC_PAGESIZE);
    
    key_t key = 0;
    GET_SYSV_KEY(key, KEY_NAME, KEY_PROJ);
    
    int sem_set_id = 0;
    SEMGET(sem_set_id, key, N_SEMS, IPC_CREAT | 0666);

    if ( CaptureBuffer(sem_set_id, WRITER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    fprintf(stderr, "Writer captured\n");
    
    int shm_id = 0;
    SHMGET(shm_id, key, bufsize, IPC_CREAT | 0666);
    printf("shm_id = %d\n", shm_id);

    char* buffer = NULL;
    SHMAT(char*, buffer, shm_id, NULL, 0);

    WriterAction(sem_set_id, buffer, bufsize);

/*

//  Calling the next function will delete all resourses. This will
//  cause the waiting processes to fail with EIDRM.
// 
//  Uncomment these lines to use this function.

    Leave(buffer, shm_id, sem_set_id); 

 */
    return 0;
}

// =========================================================

int WriterAction(int sem_id, char* buffer, long bufsize)
{
    if ( Handshake(sem_id, WRITER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    sprintf(buffer, "asdf\n");

    struct sembuf op1 = { SYNCHRONIZE_SEM, UP, SEM_UNDO };
    semop(sem_id, &op1, 1);

    return EXIT_SUCCESS;
}



