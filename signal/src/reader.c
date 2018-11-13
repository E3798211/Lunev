
#include "reader.h"

// =========================================================

static char BUFFER[BUFSIZE] = {};
static int  cur_byte        =  0;

// =========================================================

static void Sig1Handler(int signum)
{
    // printf("Sig1Handler\n");
    // printf("hello\n");
    SIG_LAST_NUM = signum;
    // printf("%d\n", SIG_LAST_NUM);
}

static void Sig2Handler(int signum)
{
    // printf("Sig2Handler\n");
    SIG_LAST_NUM = signum;
    // printf("%d\n", SIG_LAST_NUM);
}

// =========================================================

static int ReceiveByte(pid_t writer)
{
    SIG_LAST_NUM = 0;

    sigset_t sig_default_set = {};
    SIGEMPTYSET(&sig_default_set);

    for(int i = 0; i < 8; i++)
    {
        while(SIG_LAST_NUM != SIGUSR1 &&
              SIG_LAST_NUM != SIGUSR2)
            sigsuspend(&sig_default_set);

//        printf("got %s\n", sys_siglist[SIG_LAST_NUM]);

        if (SIG_LAST_NUM == SIGUSR2)
            BUFFER[cur_byte] |= (1 << i);
        else
        if (SIG_LAST_NUM == SIGUSR1)
            BUFFER[cur_byte] |= (0 << i);
        else
        // if (SIG_LAST_NUM == SIGURG)
        {
            // printf("EXIT\n");
            return EXIT_FAILURE;
        }

        // printf("%s\n", sys_siglist[SIG_LAST_NUM]);

        // Stop sending when byte is sent
        if (i != 7)
            KILL(writer, SIGUSR2);

        SIG_LAST_NUM = 0;
    }

    return EXIT_SUCCESS;
}

// =========================================================

int Reader(pid_t writer)
{
    // Parent

    struct sigaction act = {};
    act.sa_handler = Sig1Handler;
    SIGACTION(SIGUSR1, &act, NULL);
    act.sa_handler = Sig2Handler;
    SIGACTION(SIGUSR2, &act, NULL);
    act.sa_handler = Sig2Handler;
    SIGACTION(SIGHUP,  &act, NULL);

    // Starting transition
    getchar();
    kill(writer, SIGUSR1);

    while(!ReceiveByte(writer))
    {
        if (cur_byte == BUFSIZE - 1)
        {
            write(STDOUT_FILENO, BUFFER, cur_byte);
            cur_byte = 0;
        }
        else
            cur_byte++;
/*
        for(int i = 0; i < 8; i++)
        {
            printf("%d", (BUFFER[cur_byte] & (1 << i)) != 0);
        }
        printf("\n");
*/
//        printf("%c\n", BUFFER[cur_byte]);
        // Continuing transition
        printf("%c", BUFFER[cur_byte - 1]);
        KILL(writer, SIGUSR2);
    }

    // Flushing buffer
    write(STDOUT_FILENO, BUFFER, cur_byte);

    return EXIT_SUCCESS;
}

