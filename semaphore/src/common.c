
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

/*
    Entering critical section #1
 */
    SEMOP(sem_id, sem_ops, 3,
          "semop, capture");

    return EXIT_SUCCESS;
}

int Handshake(int sem_id, int caller)
{
    struct sembuf sem_ops[4] = {};
/*  
    Handshake is happening -> MUTEX must be 0. Even if process dies
    immediately after this, nothing bad happens, MUTEX must be zeroed
    anyway before transition starts.

    The worst situation is when MUTEX == 2. In this case both reader
    and wrater will succeed in reducing its value.
 */
    sem_ops[0].sem_num = MUTEX;
    sem_ops[0].sem_op  = DOWN;
    sem_ops[0].sem_flg = IPC_NOWAIT;

    errno = 0;
    if ( (semop(sem_id, sem_ops, 1) == -1) &&
          errno != EAGAIN )
    {
        perror("semop, initial down on MUTEX");
        return EXIT_FAILURE;
    }


    sem_ops[0].sem_num = ( (caller)? READER_READY_SEM : WRITER_READY_SEM );
/*
    First UP is used for syncronization, second - to let the pair check 
    if this process is alive.  
 */
    sem_ops[0].sem_op  = UP + UP;
    sem_ops[0].sem_flg = SEM_UNDO;
/*
    Avoiding deadlocks
    
    "UP and DOWN" operation never blocks and never changes MUTEX's value.
    SEM_UNDO used with only one of the operations provides with ability to 
    perform approprite operation ANYWAY. This means that process will 
    release MUTEX either alive or dead.
 */
    sem_ops[1].sem_num = MUTEX;
    sem_ops[1].sem_op  = UP;
    sem_ops[1].sem_flg = ( (caller)? SEM_UNDO : 0 );

    sem_ops[2].sem_num = MUTEX;
    sem_ops[2].sem_op  = DOWN;
    sem_ops[2].sem_flg = ( (caller)? 0 : SEM_UNDO );

    SEMOP(sem_id, sem_ops, 3,
          "semop, handshake ready");

 /*
    Waiting for the pair
  */
    sem_ops[0].sem_num = ( (caller)? WRITER_READY_SEM : READER_READY_SEM );
    sem_ops[0].sem_op  = DOWN;
    sem_ops[0].sem_flg = 0;
    
    sem_ops[1].sem_num = ( (caller)? WRITER_OTHR_SEM  : READER_OTHR_SEM  );
    sem_ops[1].sem_op  = UP;
    sem_ops[1].sem_flg = SEM_UNDO;

    SEMOP(sem_id, sem_ops, 2,
          "semop, handshake waiting");

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


