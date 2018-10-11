
/*
	Contains realizations of functions defined in "common.h"
 */

#include "common.h"

int CaptureFifo(int is_writer, int locks[3])
{
	#define LOCK 	\
	( (is_writer)? WRITER_CAPTURE_LOCK : READER_CAPTURE_LOCK )
	#define CAPTURE \
	( (is_writer)? WRITER_CAPTURE 	   : READER_CAPTURE )

	MKFIFO(LOCK, 0644);
	MKFIFO(CAPTURE, 0644);

	// Capturing
	int lock_fd          = 0;
	int capture_fd_write = 0;
	int capture_fd_read  = 0;
	while(TRUE)
	{
		// Step 1. Waiting for lock released
		while( (lock_fd = open(LOCK, O_WRONLY | O_NONBLOCK)) != -1)	
			CLOSE(lock_fd);

		// Step 2. Taking lock
		errno = 0;
		lock_fd          = open(LOCK,    O_RDONLY | O_NONBLOCK);
		capture_fd_write = open(CAPTURE, O_RDWR   | O_NONBLOCK);
		capture_fd_read  = open(CAPTURE, O_RDONLY | O_NONBLOCK);
		if (errno)
		{
			perror("open");
			exit(EXIT_FAILURE);
		}

		// Step 3. Capturing
		write(capture_fd_write, CAPTURED, CAPTURED_LEN);

		// Step 4. Check
		int  n_bytes = 0;
		char buff[CAPTURED_LEN] = {};
		ioctl(capture_fd_read, FIONREAD, &n_bytes);
		if (n_bytes != CAPTURED_LEN)	// Other process is here
		{
			read (capture_fd_read, buff, CAPTURED_LEN);
			CLOSE(capture_fd_read);
			CLOSE(capture_fd_write);
			CLOSE(lock_fd);				// MUST be last
		}
		else break;
	}

	// Successfully captured
	locks[LOCK_FD] 			= lock_fd;
	locks[CAPTURE_READ_FD] 	= capture_fd_read;
	locks[CAPTURE_WRITE_FD]	= capture_fd_write;

	#undef LOCK
	#undef CAPTURE

	return EXIT_SUCCESS;
}
