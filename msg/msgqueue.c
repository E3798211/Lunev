

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// =================================

#define BASE 10

struct msg_t
{
    long m_type;
    char m_text[1];
};

// =================================

long GetPositiveValue(char const * str);
int  ChildAction(int child_num, int msg_queue);

// =================================

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Expected one argument\n");
        return EXIT_FAILURE;
    }
    long n_processes = GetPositiveValue(argv[1]);
    if (n_processes < 0)
    {
        printf("Expected number\n");
        return EXIT_FAILURE;
    }

    // Creating new message queue
    errno = 0;
    int msg_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0644);
    if (errno)
    {
        perror("msgget");
        return EXIT_FAILURE;
    }


    for(int i = 1; i <= n_processes; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            return ChildAction(i, msg_queue);
        }
        else
        if (pid == -1)
        {
            perror("Failed to fork");
            return EXIT_FAILURE;
        }
    }
    
    // Signal to the first one
    struct msg_t msg = { 1, 'a' };
    errno = 0;
    msgsnd(msg_queue, &msg, sizeof(msg.m_text), 
           MSG_NOERROR);
    if (errno)
    {
        perror("parent, msgsnd");
        return EXIT_FAILURE;
    }

    // Waiting for the last child
    errno = 0;
    msgrcv(msg_queue, &msg, sizeof(msg.m_text), n_processes + 1,
           MSG_NOERROR);
    if (errno)
    {
        perror("parent, msgrcv");
        return EXIT_FAILURE;
    }

    msgctl(msg_queue, IPC_RMID, NULL);

    return 0;
}

// =================================

long GetPositiveValue(char const* str)
{
    char* endptr = NULL;

    errno = 0;
    long long value = strtol(str, &endptr, BASE);

    if(ERANGE == errno)                     // Out of range
    {
        printf("Value is out of range\n");
        return -1;
    }
    if(0 != *endptr)                        // Letters as a number
    {
        printf("Expected decimal number\n");
        return -1;
    }
    if(1 > value)                           // Below zero
    {
        printf("Value should be above zero\n");
        return -1;
    }

    return value;
}

int  ChildAction(int child_num, int msg_queue)
{
    // Just waiting for the activating message
    struct msg_t msg = { 1, 'a' };
    errno = 0;
    msgrcv(msg_queue, &msg, sizeof(msg.m_text), child_num,
           MSG_NOERROR);
    if (errno)
    {
        perror("child, msgrcv");
        return EXIT_FAILURE;
    }

    printf("I'm child %5d, pid = %d\n", child_num, getpid());
    fflush(stdin);

    // Activating next child
    msg.m_type = child_num + 1;
    errno = 0;
    msgsnd(msg_queue, &msg, sizeof(msg.m_text), 
           MSG_NOERROR);
    if (errno)
    {
        perror("child, msgsnd");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}






