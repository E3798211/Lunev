#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


#define BASE 10
#define LAST -1

struct msg_t
{
	long m_type;
	char m_text[1];
};

long GetPositiveValue(char const * str);
int  ChildAction(int process_num, int n_processes,
				 int msg_queue);

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Expected one argument\n");
		return EXIT_FAILURE;
	}

	long n_processes = GetPositiveValue(argv[1]);
	printf("%d\n", n_processes);

	errno = 0;
	int msg_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
	if (errno)
	{
		perror("msgget");
		return EXIT_FAILURE;
	}

	for(int i = 1; i < n_processes + 1; i++)
	{
		int pid = fork();
		if (pid == 0)			// Child
		{

			return ChildAction(i, n_processes, msg_queue);
		}
		else if (pid == -1)
		{
			perror("fork");
			return EXIT_FAILURE;
		}
	}

	printf("PARENT\n");
// delete

	return 0;
}

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

int ChildAction(int process_num, int n_processes,
				int msg_queue)
{
	// Sending a message with process number
	struct msg_t msg = { process_num, 'a' };
	msgsnd(msg_queue, &msg, sizeof(msg.m_text), IPC_NOWAIT);
	if (errno)
	{
		perror("Sending failed");
		return EXIT_FAILURE;
	}

	// Marking last one
	if (process_num == n_processes)
	{
		msg = { unsigned(LAST), 'a' };
		msgsnd(msg_queue, &msg, sizeof(msg.m_text), IPC_NOWAIT);
	}

	// Waiting for every child to be created
	while(1)
	{	
		msgrcv(msg_queue, &msg, sizeof(msg.m_text), 
		           unsigned(LAST), IPC_NOWAIT);
	}

	printf("hello %d\n", process_num);

	// Waiting for others to be printed
	while(1)
	{
		if (process_num == 1)
		{
			printf("child %d\n", process_num);
			msgrcv(msg_queue, &msg, sizeof(msg.m_text), 
		           1, IPC_NOWAIT);
			return EXIT_SUCCESS;
		}

		msgrcv(msg_queue, &msg, sizeof(msg.m_text), 
		       -(process_num - 1), IPC_NOWAIT);
		if (errno == ENOMSG)	break;
	}

	printf("child %d\n", process_num);

	errno = 0;
	msgrcv(msg_queue, &msg, sizeof(msg.m_text), 
		   process_num, IPC_NOWAIT);
	if (errno)
	{
		perror("fuck");
	}

	return EXIT_SUCCESS;
}