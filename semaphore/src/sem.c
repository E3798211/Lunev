#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define KEY_NAME "/"
#define KEY_PROJ 777

#define N_SEMS   10


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


    getchar();

    // Actually, action
    struct sembuf sem_operations[N_SEMS] = {};
    sem_operations[0].sem_num = 0;
    sem_operations[0].sem_op  = 0;  // <-- wait for sem_val to become 0
    sem_operations[0].sem_flg = SEM_UNDO;
    
    sem_operations[1].sem_num = 0;
    sem_operations[1].sem_op  = 1;  
    sem_operations[1].sem_flg = SEM_UNDO;

    semop(sem_set_id, sem_operations, 2);

    printf("%p\n", buffer);

    sprintf(buffer, "lol");
    printf("%s\n", buffer);
    sprintf(buffer, "lol");
    printf("%s\n", buffer);

    getchar();



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


