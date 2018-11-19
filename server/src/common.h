
#ifndef   COMMON_H_INCLUDED
#define   COMMON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

// =====================================================

#define BASE        10
#define MAX_BUFSIZE 1024*128
#define STD_BUFSIZE 1024*4

// =====================================================

#define MIN( x , y ) ( ((x) < (y))? (x) : (y) )
#define MAX( x , y ) ( ((x) > (y))? (x) : (y) )

#define POW( x , y )                    \
    ({                                  \
        int res = (x);                  \
        for(int i = 1; i < (y); i++)    \
            res *= (x);                 \
        res;                            \
    })

// =====================================================

#define BUFSIZE( n )    \
    MIN( 1024*POW(3, (n_processes - n)), 1024*128 )

// =====================================================
/*
    PIPE() and CLOSE() need EXIT macro to be defined
    as code to be executed in case of error.
 */

#define PIPE( FDS )                     \
    do                                  \
    {                                   \
        errno = 0;                      \
        if (pipe( (FDS) ) == -1)        \
        {                               \
            perror("pipe() failed");    \
            exit_code = EXIT_FAILURE;   \
            goto QUIT;                  \
        }                               \
    }                                   \
    while(0)

#define CLOSE( FD )                     \
    do                                  \
    {                                   \
        errno = 0;                      \
        if (close( (FD) ) == -1)        \
        {                               \
            perror("close() failed");   \
            exit_code = EXIT_FAILURE;   \
            goto QUIT;                  \
        }                               \
    }                                   \
    while(0)

// =====================================================

struct Connection
{
    int     fds[2];     // read and write fds
    char*   buffer;
    int     bufsize;
};

// =====================================================

/*
    Allocates memory for buffers
 */
int PrepareBuffers(char** buffers, 
                   struct Connection** connections,
                   int n_processes);

/*
    Returns integer converted from string.
 */
long GetPositiveValue(char const* str);

/*
    Returns maximum from fds
 */
int FindMaxFd(struct Connection** connections,
              int n_processes);

#endif // COMMON_H_INCLUDED

