
#include "reader.h"

// =========================================================

static char BUFFER[BUFSIZE] = {};
static int  cur_byte        =  0;

// =========================================================

static void SigHandler(int signum)
{
    SIG_LAST_NUM = signum;
}

/*
static void SigHandler(int signum)
{
    // printf("Sig2Handler\n");
    SIG_LAST_NUM = signum;
    // printf("%d\n", SIG_LAST_NUM);
}
 */

// =========================================================

static int ReceiveByte(pid_t writer)
{

    sigset_t sig_default_set = {};
    SIGEMPTYSET(&sig_default_set);

    for(int i = 0; i < 8; i++)
    {
        SIG_LAST_NUM = 0;

        while(SIG_LAST_NUM != SIGUSR1 &&
              SIG_LAST_NUM != SIGUSR2 &&
              SIG_LAST_NUM != SIGURG )
            sigsuspend(&sig_default_set);

//        printf("got %s\n", sys_siglist[SIG_LAST_NUM]);

        if (SIG_LAST_NUM == SIGUSR2)
            BUFFER[cur_byte] |= (1 << i);
        else
        if (SIG_LAST_NUM == SIGUSR1)
            BUFFER[cur_byte] |= (0 << i);
        else
            return EXIT_FAILURE;

        // Stop sending when byte is sent
        if (i != 7)
            KILL(writer, SIGUSR2);
    }

    return EXIT_SUCCESS;
}

// =========================================================

int Reader(pid_t writer)
{
    // Parent

    struct sigaction act = {};
    act.sa_handler = SigHandler;
    SIGACTION(SIGUSR1, &act, NULL);
    act.sa_handler = SigHandler;
    SIGACTION(SIGUSR2, &act, NULL);
    act.sa_handler = SigHandler;
    SIGACTION(SIGURG,  &act, NULL);

    // Starting transition
    KILL(writer, SIGUSR1);

    while(!ReceiveByte(writer))
    {
        if (cur_byte == BUFSIZE - 1)
        {
            write(STDOUT_FILENO, BUFFER, cur_byte);
            cur_byte = 0;
        }
        else
            cur_byte++;

        KILL(writer, SIGUSR2);
    }

    // Flushing buffer
    write(STDOUT_FILENO, BUFFER, cur_byte);

    return EXIT_SUCCESS;
}

