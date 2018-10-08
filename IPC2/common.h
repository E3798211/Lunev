
/*
	Containes functions for both Reader and Writer.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#define TRUE    1
#define FALSE   0

#define WRITER  1
#define READER  0

#define CAPTURED            "captured"
#define CAPTURED_LEN        8

#define LOCK_FD             0
#define CAPTURE_READ_FD     1
#define CAPTURE_WRITE_FD    2

// Names of the FIFOs

#define FILE_TRANSFER       "file_transfer"
#define CONTROL             "control_fifo"

#define READER_CAPTURE      "reader_capture"
#define READER_CAPTURE_LOCK "reader_capture_lock"

#define WRITER_CAPTURE      "writer_capture"
#define WRITER_CAPTURE_LOCK "writer_capture_lock"

#define MAX_NAME_LEN        "64"


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