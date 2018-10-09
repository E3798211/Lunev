
/*
	Reader receives file content from the Writer and prints
	it to the standart output.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char const *argv[])
{
	int  fds[3] = {};
	CaptureFifo(READER, fds);

	MKFIFO(READER_FINISHED, 0644);
	MKFIFO(WRITER_FINISHED, 0644);
	MKFIFO(TRANSFER, 0644);

	errno = 0;
	int writer_finished = open(WRITER_FINISHED, O_RDONLY | O_NONBLOCK);
	fcntl(writer_finished, F_SETFL, O_RDONLY);
	int reader_finished = open(READER_FINISHED, O_WRONLY);
	int transfer_fd 	= open(TRANSFER, O_RDONLY);

	open(WRITER_CAPTURE_LOCK, O_RDONLY | O_NONBLOCK);

	char buffer[BUFFER_LEN] = {};
	int  bytes_read = 0;
	while( (bytes_read = read(transfer_fd, buffer, BUFFER_LEN)) > 0 )
	{
		write(STDOUT_FILENO, buffer, bytes_read);
	}

	close(transfer_fd);

	// Signal to another end
	char  done = 1;
	write(reader_finished, &done, 1);
	read (writer_finished, &done, 1);

	return 0;
}