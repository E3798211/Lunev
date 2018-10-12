

#include <stdio.h>

int main(int argc, char const *argv[])
{
	mkfifo("/fifos/test", 0666);
	if (res != 0 && errno != EEXIST)    
    {                                   
        perror("mkfifo");               
        exit(EXIT_FAILURE);             
    }  

    errno = 0;
    open("/fifos/test", O_RDWR);

    printf("write: %d\n", );

	return 0;
}


