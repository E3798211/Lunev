
/*
	Containes functions for both Reader and Writer.
 */

#define  _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>


#define TRUE    1
#define FALSE   0

#define WRITER  1
#define READER  0


#define LOCK_FD             0
#define CAPTURE_READ_FD     1
#define CAPTURE_WRITE_FD    2

#define BUFFER_LEN          512

// Names of the FIFOs

#define TRANSFER            "fifos/transfer"
#define WRITER_FINISHED     "fifos/writer_finished"
#define READER_FINISHED     "fifos/reader_finished"

#define READER_CAPTURE      "fifos/reader_capture"
#define READER_CAPTURE_LOCK "fifos/reader_capture_lock"

#define WRITER_CAPTURE      "fifos/writer_capture"
#define WRITER_CAPTURE_LOCK "fifos/writer_capture_lock"


#define CLOSE( fd )                         \
    do                                      \
    {                                       \
        errno = 0;                          \
        if (close( fd ))                    \
        {                                   \
            perror("close");                \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)                             \


// In code all FIFOs are meant to be existing
#define MKFIFO( name , mode )               \
    do                                      \
    {                                       \
        errno = 0;                          \
        int res = mkfifo( name, mode );     \
        if (res != 0 && errno != EEXIST)    \
        {                                   \
            perror("mkfifo");               \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)                             \



int CaptureFifo(int is_writer, int locks[3]);

