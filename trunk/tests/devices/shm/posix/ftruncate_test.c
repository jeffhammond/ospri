#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

int main(int argc, char* argv[])
{
    int world_rank = 0;

    size_t size = ( argc>1 ? atoi(argv[1]) : getpagesize() ); 
    printf("%d: size = %ld \n", world_rank, (long)size);

    int fd = open("./test", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) printf("%d: open failed: %d \n", world_rank, fd);
    else      printf("%d: open succeeded: %d \n", world_rank, fd);

    if (fd>=0)
    {
        int rc = ftruncate(fd, size);
        if (rc==0) printf("%d: ftruncate succeeded \n", world_rank);
        else       printf("%d: ftruncate failed \n", world_rank);
    }

    void * ptr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (ptr==NULL) printf("%d: mmap failed \n", world_rank);
    else           printf("%d: mmap succeeded \n", world_rank);
    
    printf("%d: trying memset \n", world_rank);
    memset(ptr, 'X', size);
    printf("%d: memset succeeded \n", world_rank);

    if (fd>=0)
    {
        int rc = ftruncate(fd, 0);
        if (rc==0) printf("%d: ftruncate succeeded \n", world_rank);
        else       printf("%d: ftruncate failed \n", world_rank);
    }

    if (fd>=0)
    {
        int rc = close(fd);
        if (rc==0) printf("%d: close succeeded \n", world_rank);
        else       printf("%d: close failed \n", world_rank);
    }

    if (ptr!=NULL)
    {
        int rc = munmap(ptr, size);
        if (rc==0) printf("%d: munmap succeeded \n", world_rank);
        else       printf("%d: munmap failed \n", world_rank);
    }

    return 0;
}
