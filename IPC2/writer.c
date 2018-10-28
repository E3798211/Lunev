
/*
	Writer gets the name of the file and transfers it to
	the Reader.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Expected an argument\n");
		return EXIT_FAILURE;
	}


	int file_to_transfer = open(argv[1], O_RDONLY);
	if (errno)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	int fds[3] = {};
    int res = CaptureFifo(WRITER, fds);
    if (res != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

	MKFIFO(READER_FINISHED, 0644);
	MKFIFO(WRITER_FINISHED, 0644);
	MKFIFO(TRANSFER, 0644);

	errno = 0;
//	int reader_finished = open(READER_FINISHED, O_RDONLY | O_NONBLOCK);
//	fcntl(reader_finished, F_SETFL, O_RDONLY);
//	int writer_finished = open(WRITER_FINISHED, O_WRONLY);
	int transfer_fd     = open(TRANSFER, O_WRONLY);
	
//	open(READER_CAPTURE_LOCK, O_RDONLY | O_NONBLOCK);
    open(READER_CAPTURE, O_RDONLY | O_NONBLOCK);

	char buffer[BUFFER_LEN] = {};
	int  bytes_read = 0;
	while( (bytes_read = read(file_to_transfer, buffer, BUFFER_LEN)) > 0 )
	{
		write(transfer_fd, buffer, bytes_read);
	}

	close(transfer_fd);

	// Signal to another end
//	char  done = 1;
//	write(writer_finished, &done, 1);
//	read (reader_finished, &done, 1);

	return 0;
}

