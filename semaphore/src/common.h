
#ifndef   COMMON_H_INCLUDED
#define   COMMON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define KEY_NAME "/"
#define KEY_PROJ 777

#define READER   1
#define WRITER   0

enum SEM_OPS
{
    DOWN = -1,
    WAIT =  0,
    UP   =  1
};

enum SEMS
{
    WRITER_SELF_SEM = 0,
    WRITER_OTHR_SEM = 1,
    READER_SELF_SEM = 2,
    READER_OTHR_SEM = 3,

    WRITER_READY_SEM = 4,
    READER_READY_SEM = 5,

    MUTEX,

    N_SEMS
};

// =========================================================

/* 
    USE EXTREMELY CAREFUL! 

    KEY MUST be an existing variable. 
    It will be assigned in macro.
 */
#define GET_SYSV_KEY( KEY, KEY_FILE, KEY_PROJ ) \
    do                                          \
    {                                           \
        errno = 0;                              \
        KEY = ftok((KEY_FILE), (KEY_PROJ));     \
        if (KEY == -1)                          \
        {                                       \
            perror("ftok");                     \
            exit(EXIT_FAILURE);                 \
        }                                       \
    }                                           \
    while(0)

/* 
    USE EXTREMELY CAREFUL! 

    SEM_ID MUST be an existing variable. 
    Iw will be assigned in macro.
 */
#define SEMGET( SEM_ID, SYSV_KEY, N_SEMS, FLAGS )   \
    do                                          \
    {                                           \
        errno = 0;                              \
        SEM_ID = semget((SYSV_KEY), (N_SEMS), (FLAGS)); \
        if (SEM_ID == -1 && errno != EEXIST)    \
        {                                       \
            perror("semget");                   \
            if (errno == EINVAL)                \
                printf("Try ipcs to check if sems set exists\n"); \
            return EXIT_FAILURE;                \
        }                                       \
    }                                           \
    while(0)

/* 
    USE EXTREMELY CAREFUL! 

    SHM_ID MUST be an existing variable. 
    Iw will be assigned in macro.
 */
#define SHMGET( SHM_ID, SYSV_KEY, SIZE, FLAGS ) \
    do                                          \
    {                                           \
        errno = 0;                              \
        SHM_ID = shmget((SYSV_KEY), (SIZE), (FLAGS));   \
        if (SHM_ID == -1 && errno != EEXIST)    \
        {                                       \
            perror("shmget");                   \
            return EXIT_FAILURE;                \
        }                                       \
    }                                           \
    while(0)

/* 
    USE EXTREMELY CAREFUL! 

    BUFFER MUST be an existing pointer. 
    Iw will be assigned in macro.
 */
#define SHMAT( BUF_TYPE, BUFFER, SHM_ID, ADDR, FLAGS ) \
    do                                          \
    {                                           \
        errno = 0;                              \
        BUFFER = (BUF_TYPE)shmat((SHM_ID), (ADDR), (FLAGS));    \
        if (BUFFER == (BUF_TYPE)-1)             \
        {                                       \
            perror("shmat");                    \
            return EXIT_FAILURE;                \
        }                                       \
    }                                           \
    while(0)


#define SEMOP( SEM_ID, SOPS, NSOPS, MSG )       \
    do                                          \
    {                                           \
        errno = 0;                              \
        if ( semop((SEM_ID), (SOPS), (NSOPS)) == -1 )   \
        {                                       \
            perror( (MSG) );                    \
            return EXIT_FAILURE;                \
        }                                       \
    }                                           \
    while(0)


// =========================================================

/*
    Ensure all other processes do not need buffer any more
 */
int CaptureBuffer(int sem_id, int caller);

/*
    Signals that caller is ready and waits for the pair
 */
int Handshake(int sem_id, int caller);

/*
    Takes names of the resourses to be freed
 */
int Leave(void* buffer, int shm_id, int sem_id);

/*
    Prints values of the semaphores
 */
void PrintSems(int sem_id);

#endif // COMMON_H_INCLUDED

