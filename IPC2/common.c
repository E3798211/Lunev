
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

/*
    // First check
    int lock = 0;
    if ( (lock = open(LOCK, O_WRONLY | O_NONBLOCK)) != -1 )
    {
        return EXIT_FAILURE;
    }

    // Taking lock
    lock = open(LOCK, O_RDONLY | O_NONBLOCK);
    if (lock < 0)
    {
        perror("lock open");
        return EXIT_FAILURE;
    }
*/

    // Opening CAPTURE
    int capture_fd = 0;
    errno = 0;
    if ( (capture_fd = open(CAPTURE, O_RDWR | O_NONBLOCK)) == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    // Setting fifo size
    int new_capacity = fcntl(capture_fd, F_SETPIPE_SZ, PIPE_BUF);
    if (new_capacity < PIPE_BUF)
    {
        printf("Failed to resize fifo\n");
        exit(EXIT_FAILURE);     // Fatal error
    }

    // Capturing
    char capture_msg[PIPE_BUF] = {};
    int  written = 0;
    if ( (write(capture_fd, capture_msg, PIPE_BUF)) != PIPE_BUF )
    {
        return EXIT_FAILURE;
    }

	#undef LOCK
	#undef CAPTURE


	return EXIT_SUCCESS;
}
