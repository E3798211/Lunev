
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <inttypes.h>

#define TRUE  1
#define FALSE 0

/*
	According to "man statfs", pair "inode + filesystem_id" 
	determines file uniquely:
	"
		The general idea is that f_fsid contains some random 
		stuff such  that the  pair (f_fsid,ino) uniquely 
		determines a file.
    "

	open(), close(), read(), write(), link(), unlink()
	operations on a file descriptor do not change its inode.

	rename() can be divided into 2 situations:
	1. file stays in the same filesystem
	2. file crosses boundaries of its filesystem

	1. rename() doesn't change inode, only names and dentries are
	affected. So it is enougth to make sure that files have same 
	inodes and the are in the same fs (files may have same inodes
	in different fs)

	2. rename() crosses boundaries of the current fs. In this case
	like unlink() is called and we treat files as different as 
	writing and reading from those file decriptors will write and 
	read different blocks on a disk. 
 */
int TestFDs(int fd1, int fd2);

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		printf("Expected 3 argumants\n");
		return EXIT_FAILURE;
	}

	int fd1 = open(argv[1], O_RDWR);
	if (fd1 == -1)
	{
		perror("open(), fd1");
		return EXIT_FAILURE;
	}
	int fd2 = open(argv[1], O_RDWR);
	if (fd2 == -1)
	{
		perror("open(), fd2");
		return EXIT_FAILURE;
	}

	printf("res = %d\n", TestFDs(fd1, fd2));
	return 0;
}

int TestFDs(int fd1, int fd2)
{
	struct stat fd_stat[2] = {};
	if ( (fstat(fd1, &fd_stat[0])) == -1)
	{
		perror("fstat() for fd1");
		return -EXIT_FAILURE;
	}
	if ( (fstat(fd2, &fd_stat[1])) == -1)
	{
		perror("fstat() for fd2");
		return -EXIT_FAILURE;
	}

	// If files are already different, just quit
	if (fd_stat[0].st_ino != fd_stat[1].st_ino)
		return FALSE;

	struct statvfs vfs_info[2] = {};
    if ( fstatvfs(fd1, &vfs_info[0]) == -1 )
    {
        perror("fstatvfs() for fd1");
        return -EXIT_FAILURE;
    }
    if ( fstatvfs(fd2, &vfs_info[1]) == -1 )
    {
        perror("fstatvfs() for fd2");
        return -EXIT_FAILURE;
    }

    // If files are in different filesystems
    if (vfs_info[0].f_fsid != vfs_info[1].f_fsid)
    	return FALSE;

    return TRUE;
}


