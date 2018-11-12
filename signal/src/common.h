

# ifndef COMMON_H_INCLUDED
# define COMMON_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>
#include <signal.h>

/*
    LINUX-ONLY

    Needed for setting signal that parent is dead.
 */
#include <sys/prctl.h>

// =========================================================

extern int SIG_LAST_NUM;

#define BUFSIZE 4096

// =========================================================

#define SIGEMPTYSET( SET )                                  \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if ( sigemptyset((SET)) == -1 )                     \
        {                                                   \
            perror("sigemptyset() failed");                 \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)

#define SIGADDSET( SET, SIGNUM )                            \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if ( sigaddset((SET), (SIGNUM)) == -1 )             \
        {                                                   \
            perror("sigaddset() failed");                   \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)

#define SIGDELSET( SET, SIGNUM )                            \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if ( sigdelset((SET), (SIGNUM)) == -1 )             \
        {                                                   \
            perror("sigdelset() failed");                   \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)

#define SIGACTION( SIGNUM, ACT, OLDACT )                    \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if (sigaction((SIGNUM), (ACT), (OLDACT)))           \
        {                                                   \
            perror("sigaction() failed");                   \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)

#define SIGPROCMASK( HOW, SET, OLDSET )                     \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if (sigprocmask((HOW), (SET), (OLDSET)))            \
        {                                                   \
            perror("sigprocmask() failed");                 \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)

#define KILL( PID, SIG )                                    \
    do                                                      \
    {                                                       \
        errno = 0;                                          \
        if ( kill((PID), (SIG)) == -1)                      \
        {                                                   \
            perror("kill() failed");                        \
            return EXIT_FAILURE;                            \
        }                                                   \
    }                                                       \
    while(0)


// =========================================================

# endif // COMMON_H_INCLUDED
