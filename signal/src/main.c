
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
 /*
    https://www.gnu.org/software/libc/manual/html_node/Sigsuspend.html
 */
    // =========================================================

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

    /*
        Setting SIGUSR1 and SIGUSR2
     */
    sigset_t sig_communication_set = {};
    SIGEMPTYSET(&sig_communication_set);
    sigset_t sig_default_set       = {};
    SIGEMPTYSET(&sig_default_set);
    
    SIGADDSET(&sig_communication_set, SIGUSR1);
    SIGADDSET(&sig_communication_set, SIGUSR2);
    SIGPROCMASK(SIG_BLOCK, &sig_communication_set, &sig_default_set);

    errno = 0;
    pid_t child_pid = fork();
    if (child_pid ==  0)    // Child
    {
        Writer(parent_pid, argv[1]);
    }
    else
    if (child_pid == -1)
    {
        perror("fork() failed");
        return EXIT_FAILURE;
    }
    else                    // Parent
    {
        Reader(child_pid);
    }

    return EXIT_SUCCESS;
}

