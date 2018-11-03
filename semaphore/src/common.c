
#include "common.h"

int CaptureBuffer(int sem_id, int caller)
{
    struct sembuf sem_ops[3] = {};

    // Waiting for the self-sem to be "free"
    sem_ops[0].sem_num = ( (caller)? READER_SELF_SEM : WRITER_SELF_SEM );
    sem_ops[0].sem_op  = WAIT;
    sem_ops[0].sem_flg = SEM_UNDO;

    // Capturing self-sem
    sem_ops[1].sem_num = ( (caller)? READER_SELF_SEM : WRITER_SELF_SEM );
    sem_ops[1].sem_op  = UP;
    sem_ops[1].sem_flg = SEM_UNDO;

    // Waiting for previous caller's opponent permission
    sem_ops[2].sem_num = ( (caller)? READER_OTHR_SEM : WRITER_OTHR_SEM );
    sem_ops[2].sem_op  = WAIT;
    sem_ops[2].sem_flg = SEM_UNDO;

    errno = 0;
    if ( semop(sem_id, sem_ops, 3) == -1 )
    {
        perror("semop, capture");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int Handshake(int sem_id, int caller)
{
    // Cleaning MUTEX
    union semun 
    {
        int              val;    
        struct semid_ds *buf;    
        unsigned short  *array;  
        struct seminfo  *__buf;
    };

    union semun arg = { 0, NULL, NULL, NULL };
    errno = 0;
    semctl(sem_id, MUTEX, SETVAL, arg);
    if (errno)
    {
        perror("semctl, failed to set MUTEX to 0");
        return EXIT_FAILURE;
    }

    struct sembuf sem_ops[4] = {};

    // Signal ready
    sem_ops[0].sem_num = ( (caller)? READER_READY_SEM : WRITER_READY_SEM );
    
/*
    First UP is used for syncronization, second - to let the pair check 
    if this process is alive. 

    After handshake value of caller_READY_SEM equals 1. Linux specifies 
    SEM_UNDO to decrease the value as much as possible if it is not
    possible to substract all the operations done on the sem. So in case 
    the process dies value of the semaphore becomes 0 and the pair is able
    to react properly. 
 */
    sem_ops[0].sem_op  = UP + UP;
    sem_ops[0].sem_flg = SEM_UNDO;

    if ( semop(sem_id, sem_ops, 1) == -1 )
    {
        perror("semop, ready");
        return EXIT_FAILURE;
    }

    // Waiting for the pair
    sem_ops[0].sem_num = ( (caller)? WRITER_READY_SEM : READER_READY_SEM );
    sem_ops[0].sem_op  = DOWN;
    sem_ops[0].sem_flg = 0;
    
    // Blocking other candidates to be a pair
    sem_ops[1].sem_num = ( (caller)? WRITER_OTHR_SEM  : READER_OTHR_SEM );
    sem_ops[1].sem_op  = UP;
    sem_ops[1].sem_flg = SEM_UNDO;

/*
    Avoiding deadlocks
    
    "UP and DOWN" operation never blocks and never changes MUTEX's value.
    SEM_UNDO used with only one of the operations provides with ability to 
    perform approprite operation ANYWAY. This means that process will 
    release MUTEX either alive or dead.
 */
    sem_ops[2].sem_num = MUTEX;
    sem_ops[2].sem_op  = UP;
    sem_ops[2].sem_flg = ( (caller)? SEM_UNDO : 0 );

    sem_ops[3].sem_num = MUTEX;
    sem_ops[3].sem_op  = DOWN;
    sem_ops[3].sem_flg = ( (caller)? 0 : SEM_UNDO );

    if ( semop(sem_id, sem_ops, 4) == -1 )
    {
        perror("semop, waiting");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int Leave(void* buffer, int shm_id, int sem_id)
{
    int errors = 0;

    errors += shmdt(buffer);
    errors += shmctl(shm_id, IPC_RMID, NULL);
    errors += semctl(sem_id, 0, IPC_RMID);

    return errors;
}

void PrintSems(int sem_id)
{
    for(int i = 0; i < N_SEMS; i++)
        fprintf(stderr, "%d ", semctl(sem_id, i, GETVAL));
    printf("\n\n");
}


