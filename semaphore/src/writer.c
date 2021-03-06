
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
static int WriterAction(int sem_id, char* buffer, long bufsize,
                        int file_to_transfer);

// =========================================================

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Expected 1 argument\n");
        return EXIT_FAILURE;
    }

    errno = 0;
    int file_to_transfer = open(argv[1], O_RDONLY);
    if (errno)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    long bufsize = sysconf(_SC_PAGESIZE);
    
    key_t key = 0;
    GET_SYSV_KEY(key, KEY_NAME, KEY_PROJ);
    
    int sem_set_id = 0;
    SEMGET(sem_set_id, key, N_SEMS, IPC_CREAT | 0666);
    
/*
    Process enters critical section #1 in CaptureBuffer()
 */
    if ( CaptureBuffer(sem_set_id, WRITER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    int shm_id = 0;
    SHMGET(shm_id, key, bufsize, IPC_CREAT | 0666);
    
    char* buffer = NULL;
    SHMAT(char*, buffer, shm_id, NULL, 0);

    int res = WriterAction(sem_set_id, buffer, bufsize,
                           file_to_transfer);

/*
    End of the critical section #1
 */
    return res;
}

// =========================================================

static int WriterAction(int sem_id, char* buffer, long bufsize, 
                        int file_to_transfer)
{
    if ( Handshake(sem_id, WRITER) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    struct sembuf sync_ops[4] = 
    {
        // Check
        { READER_READY_SEM, DOWN, IPC_NOWAIT },
        { READER_READY_SEM,   UP, IPC_NOWAIT },

        // Operations
        { MUTEX, WAIT, 0 },
        { MUTEX,   UP, 0 },
    };

    while(1)
    {
        errno = 0;
        /*
            Entering critical section #2
         */
        semop(sem_id, sync_ops, 3);
        if (errno == EAGAIN)    return EXIT_FAILURE;
        else
        if (errno)
        {
            perror("semop, waiting for write permission failed");
            return EXIT_FAILURE;
        }

        int n_bytes = read(file_to_transfer, buffer + sizeof(size_t), 
                           bufsize - sizeof(size_t));
        ((size_t*)buffer)[0] = (size_t)n_bytes;
        if (n_bytes <= 0)       return EXIT_SUCCESS;

        /*
            Leaving critical section #2
         */
        SEMOP(sem_id, sync_ops + 3, 1,
              "semop, giving read permission");
    }

    return EXIT_FAILURE;
}



