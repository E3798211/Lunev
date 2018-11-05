
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
static int ReaderAction(int sem_id, char* buffer, long bufsize);

// =========================================================

int main(int argc, char const *argv[])
{
    long bufsize = sysconf(_SC_PAGESIZE);

    key_t key = 0;
    GET_SYSV_KEY(key, KEY_NAME, KEY_PROJ);

    int sem_set_id = 0;
    SEMGET(sem_set_id, key, N_SEMS, IPC_CREAT | 0666);

    if ( CaptureBuffer(sem_set_id, READER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    int shm_id = 0;
    SHMGET(shm_id, key, bufsize, IPC_CREAT | 0666);
    
    char* buffer = NULL;
    SHMAT(char*, buffer, shm_id, NULL, 0);

    int res = ReaderAction(sem_set_id, buffer, bufsize);

/*

//  Calling the next function will delete all resourses. This will
//  cause the waiting processes to fail with EIDRM.
// 
//  Uncomment these lines to use this function.

    Leave(buffer, shm_id, sem_set_id); 

 */

    return res;
}

// =========================================================

static int ReaderAction(int sem_id, char* buffer, long bufsize)
{
    if ( Handshake(sem_id, READER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    struct sembuf sync_ops[4] = 
    {
        // Operations
        { MUTEX, DOWN, 0 },
        { MUTEX,   UP, 0 },

        // Check
        { WRITER_READY_SEM, DOWN, IPC_NOWAIT },
        { WRITER_READY_SEM,   UP, IPC_NOWAIT },
    };

    while(1)
    {
        SEMOP(sem_id, sync_ops, 2, 
              "semop, waiting for read permission");

        size_t n_bytes = ((size_t*)buffer)[0];
        write(STDOUT_FILENO, buffer + sizeof(size_t), n_bytes);

        // Checking if opponent is alive
        errno = 0;
        semop(sem_id, sync_ops + 2, 2);
        if (errno == EAGAIN)    return EXIT_SUCCESS;
        else
        if (errno)
        {
            perror("semop, check failed");
            return EXIT_FAILURE;
        }

        SEMOP(sem_id, sync_ops, 1,
              "semop, giving write permission");
    }

    return EXIT_FAILURE;
}


