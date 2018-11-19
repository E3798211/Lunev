
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/select.h>

#include "common.h"

/*
    Performs child action.
 */
int ChildAction(int fds[2]);

/*
    Performs parent action.
 */
int ParentAction(struct Connection* connections,
                 int n_processes);

// =====================================================

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("Invalid input.\n");
        printf("Usage: %s <n_processes> <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n_processes = GetPositiveValue(argv[1]);
    if  (n_processes < 0)
        return EXIT_FAILURE;

    errno = 0;
    int file_to_transfer = open(argv[2], O_RDONLY);
    if (file_to_transfer == -1)
    {
        perror("open() failed");
        return EXIT_FAILURE;
    }

    char* buffers = NULL;
    struct Connection* connections = NULL;
    if (PrepareBuffers(&buffers, &connections, n_processes))
        return EXIT_FAILURE;


    // Spawning children
    int exit_code = 0;
    for(int i = 0; i < n_processes; i++)
    {
        int parent_in_fds [2] = {};
        PIPE(parent_in_fds);
        int parent_out_fds[2] = {};
        PIPE(parent_out_fds);

        errno = 0;
        pid_t pid = fork();
        if (pid == 0)   // Child
        {
            CLOSE(parent_in_fds [0]);
            CLOSE(parent_out_fds[1]);
            
            int child_fds[2] = 
            {
                ( (!i)?   file_to_transfer : parent_out_fds[0] ),
                parent_in_fds[1]
            };
            
            exit_code = ChildAction(child_fds);
            goto QUIT;
        }
        else
        if (pid == -1)  // Error
        {
            perror("fork() failed");
            exit_code = EXIT_FAILURE;
            goto QUIT;
        }

        // Setting parent's connections
        CLOSE(parent_in_fds [1]);
        CLOSE(parent_out_fds[0]);

        connections[i].fds[0] = parent_in_fds [0];
        connections[i].fds[1] = parent_out_fds[1];
        connections[i].buffer = buffers + MAX_BUFSIZE * i;
        connections[i].bufsize = BUFSIZE(i);
    }

    exit_code = ParentAction(connections, n_processes);

QUIT:
    free(connections);
    free(buffers);
    return exit_code;
}

// =====================================================

int ChildAction(int fds[2])
{
    char buffer[STD_BUFSIZE] = {};

    int bytes_read = 0;
    while( (bytes_read = read(fds[0], buffer, STD_BUFSIZE)) > 0 )
    {
        // Sending all the data that has been read
        int bytes_written = 0;
        while(bytes_written < bytes_read)
        {
            errno = 0;
            int bytes = write(fds[1], buffer + bytes_written, 
                              bytes_read - bytes_written);
            if (bytes == -1)
            {
                perror("write() failed");
                return EXIT_FAILURE;
            }

            bytes_written += bytes;
        }

    }
    return EXIT_SUCCESS;
}

int ParentAction(struct Connection* connections,
                 int n_processes)
{
    if (!connections)   return EXIT_FAILURE;

    int nfds = FindMaxFd(connections, n_processes) + 1;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    fd_set write_fds;
    FD_ZERO(&write_fds);
    for(int i = 0; i < n_processes; i++)
    {
        FD_SET(connections[i].fds[0], &read_fds);
        FD_SET(connections[i].fds[1], &write_fds);
    }

    struct timeval timeout = { 0, 1000 };

    // SELECT + READ AND SEND

    return EXIT_FAILURE;
}

