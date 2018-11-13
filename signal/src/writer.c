
#include "writer.h"

// =========================================================

static char BUFFER[BUFSIZE] = {};
static int  cur_byte        =  0;

// =========================================================

static void Sig1Handler(int signum)
{
//    printf("Sig1Handler\n");
    SIG_LAST_NUM = signum;
//    printf("%d\n", SIG_LAST_NUM);
}

static void Sig2Handler(int signum)
{
//    printf("Sig2Handler\n");
    SIG_LAST_NUM = signum;
//    printf("%d\n", SIG_LAST_NUM);
}

// =========================================================

static int SendByte(pid_t reader)
{

    sigset_t sig_default_set = {};
    SIGEMPTYSET(&sig_default_set);

    for(int i = 0; i < 8; i++)
    {
        SIG_LAST_NUM = 0;
        // Sending bit
        if ( (BUFFER[cur_byte] & (1 << i)) == 0 )
            KILL(reader, SIGUSR1);
        else
            KILL(reader, SIGUSR2);

        // Waiting for write permission
        while(SIG_LAST_NUM != SIGUSR2)
            sigsuspend(&sig_default_set);
    }

    return EXIT_SUCCESS;
}

// =========================================================

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

    // Dying if parent is dead
    errno = 0;
    if ( prctl(PR_SET_PDEATHSIG, SIGKILL) == -1 )
    {
        perror("prctl() failed");
        return EXIT_FAILURE;
    }
    /*
        Parent could become dead before signal is set. So after
        signal is set, checking if parent is still alive.

        After next check we are not afraid of parent's death as
        it raises SIGKILL.
     */
    if (getppid() != reader)        return EXIT_FAILURE;

    struct sigaction act = {};
    act.sa_handler = Sig1Handler;
    SIGACTION(SIGUSR1, &act, NULL);
    act.sa_handler = Sig2Handler;
    SIGACTION(SIGUSR2, &act, NULL);

    sigset_t sig_default_set = {};
    SIGEMPTYSET(&sig_default_set);

    // Waiting for parent is ready
    while(SIG_LAST_NUM != SIGUSR1)
        sigsuspend(&sig_default_set);

    // If we are here, file can be transmitted
    int bytes = 0;
    while( (bytes = read(file_to_transfer, BUFFER, BUFSIZE)) > 0 )
        for(cur_byte = 0; cur_byte < bytes; cur_byte++)
            if (SendByte(reader))   return EXIT_FAILURE;

    // SIGNAL END
    KILL(reader, SIGURG);

    /*
        Reader receives all the bytes, prints and qiuts ->
        writer receives SIGKILL
     */
    pause();

    return EXIT_SUCCESS; 
}

