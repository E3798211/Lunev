
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

    // Whaiting for previous caller's opponent permission
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
    struct sembuf sem_ops[2] = {};

    // Signal ready
    sem_ops[0].sem_num = ( (caller)? READER_READY_SEM : WRITER_READY_SEM );
    sem_ops[0].sem_op  = UP;
    sem_ops[0].sem_flg = SEM_UNDO;

    if ( semop(sem_id, sem_ops, 1) == -1 )
    {
        perror("semop, ready");
        return EXIT_FAILURE;
    }

    // Waiting for the pair
    sem_ops[0].sem_num = ( (caller)? WRITER_READY_SEM : READER_READY_SEM );
    sem_ops[0].sem_op  = DOWN;
    sem_ops[0].sem_flg = SEM_UNDO;

    sem_ops[1].sem_num = ( (caller)? WRITER_OTHR_SEM : READER_OTHR_SEM );
    sem_ops[1].sem_op  = UP;
    sem_ops[1].sem_flg = SEM_UNDO;

    if ( semop(sem_id, sem_ops, 2) == -1 )
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

    errno = 0;
    errors += semctl(sem_id, 0, IPC_RMID);
    if (errno != 0)
    {
        perror("semctl, delete");
    }

    return errors;
}

