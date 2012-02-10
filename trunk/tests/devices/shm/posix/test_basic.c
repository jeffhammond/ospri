#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

/*
int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);
*/

int main(int argc, char * argv[])
{
    long size = ( argc>1 ? atol(argv[1]) : (long)getpagesize() ); 
    printf("size = %ld \n", size);

    int fd = shm_open("./bar", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
    if (fd<0) printf("shm_open failed: %d \n", fd);
    else      printf("shm_open succeeded: %d \n", fd);

    //int fd = open("/dev/shm/foo", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
    //if (fd<0) printf("open failed: %d \n", fd);
    //else      printf("open succeeded: %d \n", fd);

    int rc = close(fd);
    if (rc==0) printf("close succeeded \n");
    else       printf("close failed \n");

    return 0;
}

