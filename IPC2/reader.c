
/*
	This program consumes text through FIFO and places
	it to the stdout
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define MAIN_FIFO_PATH		"/home/mint/MIPT/fifos/MAINFIFO"
#define FIFO_PATH			"/home/mint/MIPT/fifos/"
#define FIFO_PATH_LENGTH	31
#define BUFFER_SIZE 		1024
#define NAME_LEN			8
#define BASE				10


int GetUniqueName	(char name[NAME_LEN]);
int TransferFile	(int source, int dest);

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Expected one filename as an argument\n");
		return EXIT_FAILURE;
	}
	if (access(argv[1], F_OK))
	{
		perror("access");
		return EXIT_FAILURE;
	}

	// Preparing file to be transfered
	int to_be_transfered = open(argv[1], O_RDONLY);
	if (to_be_transfered < 0)
	{
		perror("opening file");
		return EXIT_FAILURE;
	}

	// Creating main pipe if innocent
	errno = 0;
	int main_fifo = mkfifo(MAIN_FIFO_PATH, 0666);
	if (main_fifo != 0 && errno != EEXIST)
	{
		perror("mkfifo, main fifo");
		return EXIT_FAILURE;
	}

	errno = 0;
	main_fifo = open(MAIN_FIFO_PATH, O_RDONLY);
	if (main_fifo < 0)
	{
		perror("open, main fifo");
		return EXIT_FAILURE;
	}

	char name[NAME_LEN] = {};
	
	// POSIX gauranties read and write to be atomic.
	// NAME_LEN must be 2^n to fill FIFO correctly
	errno = 0;
	read(main_fifo, name, NAME_LEN);
	if (errno)
	{
		perror("write, main fifo");
		return EXIT_FAILURE;
	}

	// Do not need and access to the MAIN_FIFO any more
	close(main_fifo);

	// Opening local fifo
	char   local_fifo_name[FIFO_PATH_LENGTH] = FIFO_PATH;
	strcat(local_fifo_name, name);
	local_fifo_name[FIFO_PATH_LENGTH - 1] = '\0';
	
	int local_fifo = -1;
	for(size_t i = 0; i < UINT_MAX; i++)
	{
		open(local_fifo_name, O_WRONLY | O_NONBLOCK);
		if (local_fifo < 0)
		{
			perror("open, local fifo");
			return EXIT_FAILURE;
		}
	}

	// ACTION
	int status = TransferFile(to_be_transfered, local_fifo);
	close(to_be_transfered);
	close(local_fifo);
	
	return status;
}

// =================================================

int GetUniqueName	(char name[NAME_LEN])
{
	int pid = getpid();
	for(int i = 0; i < NAME_LEN; i++)
	{
		int digit = pid % BASE;
		name[NAME_LEN - 1 - i] = digit + '0';
		pid = pid / BASE;
	}

	return 0;
}

int TransferFile	(int source, int dest)
{
	char buffer[BUFFER_SIZE] = {};
	int  bytes_read = 0;
	while( (bytes_read = read(source, buffer, BUFFER_SIZE)) > 0 )
	{
		write(dest, buffer, bytes_read);
		if (errno)
		{
			perror("write, final output");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

