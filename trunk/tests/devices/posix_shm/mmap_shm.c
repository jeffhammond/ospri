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
    if (fd<0) fprintf(stderr,"shm_open failed: %d \n", fd);
    else      fprintf(stderr,"shm_open succeeded: %d \n", fd);
#elif defined(DEV_SHM)
    int fd = open("/dev/shm/foo", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) fprintf(stderr,"open failed: %d \n", fd);
    else      fprintf(stderr,"open succeeded: %d \n", fd);
#else
    int fd = -1;
    fprintf(stderr,"no file backing \n");
#endif

    if (fd>=0)
    {
        int rc = ftruncate(fd, size);
        if (rc==0) fprintf(stderr,"ftruncate succeeded \n");
        else       fprintf(stderr,"ftruncate failed \n");
    }

    void * ptr = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (ptr==NULL) fprintf(stderr,"mmap failed \n");
    else           fprintf(stderr,"mmap succeeded \n");
    
    memset(ptr, '\0', size);
    fprintf(stderr,"memset succeeded \n");

#if defined(POSIX_SHM)
    if (fd>=0)
    {
        int rc = shm_unlink("/bar");
        if (rc==0) fprintf(stderr,"shm_unlink succeeded \n");
        else       fprintf(stderr,"shm_unlink failed \n");
    }
#elif defined(DEV_SHM)
    if (fd>=0)
    {
        int rc = -1;

        rc = ftruncate(fd, 0);
        if (rc==0) fprintf(stderr,"ftruncate succeeded \n");
        else       fprintf(stderr,"ftruncate failed \n");

        rc = close(fd);
        if (rc==0) fprintf(stderr,"close succeeded \n");
        else       fprintf(stderr,"close failed \n");
    }
#endif

    if (ptr!=NULL)
    {
        int rc = munmap(ptr, size);
        if (rc==0) fprintf(stderr,"munmap succeeded \n");
        else       fprintf(stderr,"munmap failed \n");
    }

    return 0;
}
