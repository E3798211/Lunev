
#include "common.h"
#include "reader.h"
#include "writer.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// =========================================================

int SIG_LAST_NUM = 0;

// =========================================================

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Expected 2 arguments\n");
        return EXIT_FAILURE;
    }

    /* 
        Get parent's pid - used later to check if parent is alive.
     */
    pid_t parent_pid = getpid();

    /*
        Setting SIGCHLD to be deadly.
     */
    struct sigaction act = {};
    act.sa_handler = exit;
    act.sa_flags   = SA_NOCLDWAIT;
    SIGACTION(SIGCHLD, &act, NULL);

    act.sa_handler = SigHandler;
    SIGACTION(SIGUSR1, &act, NULL);
    SIGACTION(SIGUSR2, &act, NULL);
    SIGACTION(SIGURG,  &act, NULL);

    /*
        Setting SIGUSR1 and SIGUSR2
     */
    sigset_t sig_communication_set = {};
    SIGEMPTYSET(&sig_communication_set);
    
    SIGADDSET(&sig_communication_set, SIGUSR1);
    SIGADDSET(&sig_communication_set, SIGUSR2);
    SIGADDSET(&sig_communication_set, SIGURG );
    SIGPROCMASK(SIG_BLOCK, &sig_communication_set, NULL);

    errno = 0;
    int exit_status = 0;
    pid_t child_pid = fork();
    if (child_pid ==  0)    // Child
    {
        exit_status = Writer(parent_pid, argv[1]);
    }
    else
    if (child_pid == -1)
    {
        perror("fork() failed");
        return EXIT_FAILURE;
    }
    else                    // Parent
    {
        exit_status = Reader(child_pid);
    }

    return exit_status;
}

