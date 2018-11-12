
#include "writer.h"

static void go(int signum)
{
    printf("hi\n");
    SIG_LAST_NUM = signum;

}

int Writer(pid_t reader, char const* filename)
{
    // Child

    int file_to_transfer = open(filename, O_RDONLY);
    errno = 0;
    if (file_to_transfer == -1)
    {
        perror("open() failed");
        return EXIT_FAILURE;
    }

    // Dying when parent is dead
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    /*
        Parent could become dead before signal is set. So after
        signal is set, checking if parent is still alive.

        After next check we are not afraid of parent's death as
        it raises SIGKILL.
     */
    if (getppid() != reader)        return EXIT_FAILURE;

    // Waiting for parent is ready
    struct sigaction act = {};
    act.sa_handler = go;
    SIGACTION(SIGUSR1, &act, NULL);

    sigset_t sig_default_set = {};
    SIGEMPTYSET(&sig_default_set);

    while(SIG_LAST_NUM != SIGUSR1)
        sigsuspend(&sig_default_set);

    // If we are here, file can be transmitted
    printf("HELL YEAH\n");
    
    


    return EXIT_SUCCESS; 
}

