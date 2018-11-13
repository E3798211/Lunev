
#include "reader.h"

// =========================================================

static char BUFFER[BUFSIZE] = {};
static int  cur_byte        =  0;

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

        if (SIG_LAST_NUM == SIGUSR2)
            BUFFER[cur_byte] |=  (1 << i);
        else
        if (SIG_LAST_NUM == SIGUSR1)
            BUFFER[cur_byte] &= ~(1 << i);
        else
            return EXIT_FAILURE;

        // Stop writer when whole byte is sent
        if (i != 7)
            KILL(writer, SIGUSR2);
    }

    return EXIT_SUCCESS;
}

// =========================================================

int Reader(pid_t writer)
{
    // Parent

    while(!ReceiveByte(writer))
    {
        if (cur_byte == BUFSIZE - 1)
        {
            write(STDOUT_FILENO, BUFFER, BUFSIZE);
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

