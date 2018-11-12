
# ifndef WRITER_H_INCLUDED
# define WRITER_H_INCLUDED

#include "common.h"

/*
    Opens file and transfers it to the reader.

    Child.
 */
int Writer(pid_t reader, char const* filename);

# endif // WRITER_H_INCLUDED
