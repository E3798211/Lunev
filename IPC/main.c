
/*
	Process creates child, child sends file content to the parent,
	parent prints it
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1024

#define TRUE  1
#define FALSE 0

int ChildAction (int fds[2], char* filename);
int ParentAction(int fds[2], int child_pid);

int main(int argc, char* argv[])
{
	// Instant check
	if(argc != 2)
	{
		printf("Expected filename as an argument\n");
		return EXIT_FAILURE;
	}

	// Creating pipe
	int fds[2] = {};
	if(pipe(fds))
	{
		perror("pipe");
		return EXIT_FAILURE;
	}

	// Creating child
	int pid = fork();
	if (pid == 0)		// Child
	{
		return ChildAction(fds, argv[1]);
	}
	else if(pid == -1)	// Error
	{
		perror("fork");
		return EXIT_FAILURE;
	}
	else				// Parent
	{
		return ParentAction(fds, pid);
	}

	return 0;
}



int ChildAction (int fds[2], char* filename)
{
	close(fds[0]);		// Child only writes

	int  file = -1;
	if( (file = open(filename, O_RDONLY)) < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	// Transfering file
	char buffer[BUFFER_SIZE] = {};
	while(TRUE)
	{
		int bytes_read = read(file, buffer, BUFFER_SIZE);
		if (errno != 0)
		{
			perror("read");
			return EXIT_FAILURE;
		}
		if(bytes_read == 0)		break;	// EOF

		int bytes_written = write(fds[1], buffer, bytes_read);
		if (errno != 0)
		{
			perror("write");
			return EXIT_FAILURE;
		}
	}

	close(fds[1]);
	close(file);

	return 0;
}

int ParentAction(int fds[2], int child_pid)
{
	close(fds[1]);		// Parent only reads

	char buffer[BUFFER_SIZE] = {};
	while(TRUE)
	{
		int bytes_read = read(fds[0], buffer, BUFFER_SIZE);
		if (bytes_read <= 0)	break;	// EOF

		write(STDOUT_FILENO, buffer, bytes_read);
	}

	close(fds[0]);

	return 0;
}
