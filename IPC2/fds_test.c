
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
	// 
	return 0;
}

int TestFDs(int fd1, int fd2)
{
	struct stat fd1_stat = {};
	if ( (fstat(fd1, &fd1_stat)) == -1)
	{
		perror("stat for fd1");
		return -1;
	}

	struct stat fd2_stat = {};
	if ( (fstat(fd2, &fd2_stat)) == -1)
	{
		perror("stat for fd2");
		return -1;
	}


}