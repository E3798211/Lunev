
/*
	This program produces text to put it through FIFO

	Programm expects filename as an argument
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
int RecieveFile		(int source);

int main(int argc, char const *argv[])
{
	char name[NAME_LEN] = {};
	GetUniqueName(name);

	// Create output FIFO with correct name
	char   local_fifo_name[FIFO_PATH_LENGTH] = FIFO_PATH;
	strcat(local_fifo_name, name);
	local_fifo_name[FIFO_PATH_LENGTH - 1] = '\0';
	int local_fifo = mkfifo(local_fifo_name, 0666);
	if (local_fifo != 0 && errno != EEXIST)
	{
		perror("mkfifo, local fifo");
		return EXIT_FAILURE;
	}
	// Open local FIFO
	local_fifo = open(local_fifo_name, O_RDONLY | O_NONBLOCK);
	if (local_fifo < 0)
	{
		perror("open, local fifo");
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
	main_fifo = open(MAIN_FIFO_PATH, O_WRONLY);
	if (main_fifo < 0)
	{
		perror("open, main fifo");
		return EXIT_FAILURE;
	}

	// Put name to main pipe
	write(main_fifo, name, NAME_LEN);
	if (errno)
	{
		perror("write, main fifo");
		return EXIT_FAILURE;
	}

	close(main_fifo);
/*
	// Wait
	char buffer[BUFFER_SIZE] = {};
	for(size_t i = 0; i < UINT_MAX; i++)
	{
		write(local_fifo, buffer, 0);
		if (errno != EAGAIN && errno != EWOULDBLOCK)	break;
	}
*/
	// Action
	int status = RecieveFile(local_fifo);
	close (local_fifo);
	remove(local_fifo_name);

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

int RecieveFile		(int source)
{
	char buffer[BUFFER_SIZE] = {};
	int  bytes_read = 0;
	while( (bytes_read = read(source, buffer, BUFFER_SIZE)) > 0 )
	{
		write(STDOUT_FILENO, buffer, bytes_read);
		if (errno)
		{
			perror("write, final output");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

