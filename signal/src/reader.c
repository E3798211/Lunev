
#include "reader.h"

int Reader(pid_t writer)
{
    // Parent

    printf("parent\n");

    usleep(1000000);
    kill(writer, SIGUSR1);

    pause();

    return EXIT_SUCCESS;
}

