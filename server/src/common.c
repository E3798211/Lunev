
#include "common.h"

int PrepareBuffers(char** buffers, 
                   struct Connection** connections,
                   int n_processes)
{
    if (!buffers || !connections)
        return EXIT_FAILURE;

    errno = 0;
    *buffers =
    (char*)calloc(n_processes, sizeof(char)*MAX_BUFSIZE);
    if (!buffers)
    {
        perror("calloc() buffer");
        return EXIT_FAILURE;
    }
    
    errno = 0;
    *connections = 
    (struct Connection*)calloc(n_processes, sizeof(struct Connection));
    if (!connections)
    {
        perror("calloc() connections");
        free(buffers);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

long GetPositiveValue(char const* str)
{
    if (!str)
        return -1;

    char* endptr = NULL;

    errno = 0;
    long long value = strtol(str, &endptr, BASE);

    if(ERANGE == errno)                     // Out of range
    {
        printf("Value is out of range\n");
        return -1;
    }
    if(0 != *endptr)                        // Letters as a number
    {
        printf("Expected decimal number\n");
        return -1;
    }
    if(1 > value)                           // Below zero
    {
        printf("Value should be above zero\n");
        return -1;
    }

    return value;
}

int FindMaxFd(struct Connection* connections,
              int n_processes)
{
    if (!connections)   return EXIT_FAILURE;

    int max = 0;
    for(int i = 0; i < n_processes; i++)
    {
        max = MAX(max, connections[i].fds[0]);
        max = MAX(max, connections[i].fds[1]);
    }

    return max;
}


