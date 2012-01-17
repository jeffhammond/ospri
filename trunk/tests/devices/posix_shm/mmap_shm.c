#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

//#define DEV_SHM
#define POSIX_SHM

int main(int argc, char* argv[])
{
    size_t size = ( argc>1 ? atoi(argv[1]) : getpagesize() ); 
    printf("size = %ld \n", (long)size);

#if defined(POSIX_SHM)
    int fd = shm_open("/bar", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) printf("shm_open failed: %d \n", fd);
    else      printf("shm_open succeeded: %d \n", fd);
#elif defined(DEV_SHM)
    int fd = open("/dev/shm/foo", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) printf("open failed: %d \n", fd);
    else      printf("open succeeded: %d \n", fd);
#else
    int fd = -1;
    printf("no file backing \n");
#endif

    if (fd>=0)
    {
        int rc = ftruncate(fd, size);
        if (rc==0) printf("ftruncate succeeded \n");
        else       printf("ftruncate failed \n");
    }

    void * ptr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (ptr==NULL) printf("mmap failed \n");
    else           printf("mmap succeeded \n");
    
    printf("trying memset \n");
    memset(ptr, '\0', size);
    printf("memset succeeded \n");

    if (fd>=0)
    {
        int rc = ftruncate(fd, 0);
        if (rc==0) printf("ftruncate succeeded \n");
        else       printf("ftruncate failed \n");
    }

#if defined(POSIX_SHM)
    if (fd>=0)
    {
        int rc = shm_unlink("/bar");
        if (rc==0) printf("shm_unlink succeeded \n");
        else       printf("shm_unlink failed \n");
    }
#elif defined(DEV_SHM)
    if (fd>=0)
    {
        int rc = close(fd);
        if (rc==0) printf("close succeeded \n");
        else       printf("close failed \n");
    }
#endif

    if (ptr!=NULL)
    {
        int rc = munmap(ptr, size);
        if (rc==0) printf("munmap succeeded \n");
        else       printf("munmap failed \n");
    }

    return 0;
}
