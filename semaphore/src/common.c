
#include "common.h"

int Leave(void* buffer, int sem_id, int shm_id)
{
    int errors = 0;

    errors += shmdt(buffer);
    errors += semctl(sem_id, 0, IPC_RMID);
    errors += shmctl(shm_id, IPC_RMID, NULL);

    return errors;
}