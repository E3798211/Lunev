
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <limits.h>

#include <sys/select.h>

#include "common.h"

/*
    Performs child action.
 */
int ChildAction(int fds[2], int num);

/*
    Performs parent action.
 */
int ParentAction(struct Connection* connections,
                 int n_processes);

/*
    Reads and writes from buffers
 */
static int ProcessConnections(struct Connection* connections,
            fd_set* read_fds, fd_set* write_fds,
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

    struct sigaction sa = {};
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);

/*
    In main() every return hereinafter involves freeing
    allocated resourses, so EXIT must be defined as following
 */
#define EXIT    exit_code = EXIT_FAILURE; goto QUIT;

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

            /*
                Closing unnecessary connections: children shouldn't
                keep each other's file descriptors opened.
             */
            for(int j = 0; j < i; j++)
            {
                CLOSE(connections[j].fds[0]);
                CLOSE(connections[j].fds[1]);
            }
 
            int child_fds[2] = 
            {
                ( (!i)?   file_to_transfer : parent_out_fds[0] ),
                parent_in_fds[1]
            };

            exit_code = ChildAction(child_fds, i);
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
    free(buffers);
    free(connections);

    return exit_code;

#undef EXIT
}

// =====================================================

#define EXIT return EXIT_FAILURE;

int ChildAction(int fds[2], int num)
{
    char buffer[STD_BUFSIZE] = {};

    int bytes_read = 0;
    while( (bytes_read = read(fds[0], buffer, STD_BUFSIZE)) > 0 )
    {
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
    fd_set write_fds;
    struct timeval timeout;

    close(connections[0].fds[1]);
    connections[0].fds[1] = CLOSED;

    errno = 0;
    for(int i = 1; i < n_processes; i++)
    {
        int flag = 0;
        flag = fcntl(connections[i].fds[0], F_GETFL);
        if (fcntl(connections[i].fds[0], F_SETFL, O_NONBLOCK) == -1)
        {
            perror("fcntl() failed");
            return EXIT_FAILURE;
        }
        flag = fcntl(connections[i].fds[1], F_GETFL);
        if (fcntl(connections[i].fds[1], F_SETFL, O_NONBLOCK) == -1)
        {
            perror("fcntl() failed");
            return EXIT_FAILURE;
        }
    }

    while(1)
    {
        // Initing arguments for select
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        for(int i = 0; i < n_processes; i++)
        {
            if (connections[i].fds[0] != CLOSED)
                FD_SET(connections[i].fds[0], &read_fds);
            if (connections[i].fds[1] != CLOSED)
                FD_SET(connections[i].fds[1], &write_fds);
        }

        timeout.tv_sec  = WAIT_TIME_SEC;
        timeout.tv_usec = WAIT_TIME_USEC;

        int ready = select(nfds, &read_fds, &write_fds, 
                           NULL, &timeout);
        if (ready < 0)
        {
            perror("select() failed");
            return EXIT_FAILURE;
        }

        int res = ProcessConnections(connections,
                    &read_fds, &write_fds, n_processes);
        if (res != 0)               return EXIT_FAILURE;

        // Writing to stdout
        struct Connection last = connections[n_processes - 1];
        
        while(last.left > 0)
        {
            int bytes = write(STDOUT_FILENO, last.buffer + last.offset,
                              last.left);
            if (bytes < 0)              return EXIT_FAILURE;

            last.left   -= bytes;
            last.offset += bytes;
        }

        if (last.fds[0] == CLOSED && last.left == 0)
        {
            close(last.fds[1]);
            last.fds[1] = CLOSED;

            return EXIT_SUCCESS;
        }

        connections[n_processes - 1] = last;
    }

    return EXIT_SUCCESS;
}

static int ProcessConnections(struct Connection* connections,
            fd_set* read_fds, fd_set* write_fds,
            int n_processes)
{
    if (!connections || !read_fds || !write_fds)    
        return EXIT_FAILURE;


    // Read
    for(int i = 0; i < n_processes; i++)
    {
        struct Connection cur  = connections[i];

        if (cur.fds[0] != CLOSED &&
            FD_ISSET(cur.fds[0], read_fds) &&
                     cur.left == 0)
        {
            errno = 0;
            ssize_t bytes = read(cur.fds[0], cur.buffer, cur.bufsize);
            if (bytes <  0)
            {
                perror("read() failed");
                return EXIT_FAILURE;
            }
            
            // Whole file is read, connection can be closed 
            if (bytes == 0)
            {
                CLOSE(cur.fds[0]);
                cur.fds[0] = CLOSED;
            }

            cur.left   = bytes;
            cur.offset = 0;
        }

        connections[i] = cur;
    }

    // Write
    for(int i = 1; i < n_processes; i++)
    {
        struct Connection cur  = connections[i];
        struct Connection prev = connections[i - 1];

        if (cur.fds[1] != CLOSED &&
            FD_ISSET(cur.fds[1], write_fds))
        {
            errno = 0;
            int bytes = write(cur.fds[1], prev.buffer + prev.offset,
                              prev.left);
            if (bytes < 0)
            {
                perror("write() failed");
                return EXIT_FAILURE;
            }
            
            prev.left   -= bytes;
            prev.offset += bytes;

            // Whole file transmitted, connection can be closed
            if (prev.fds[0] == CLOSED && prev.left == 0)
            {
                CLOSE(cur.fds[1]);
                cur.fds[1] = CLOSED;
            }
        }

        connections[i]     = cur;
        connections[i - 1] = prev;
    }

    return EXIT_SUCCESS;
}

#undef EXIT

