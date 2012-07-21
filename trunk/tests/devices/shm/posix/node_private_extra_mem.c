#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <mpi.h>

#ifdef __bgp__
#  include <mpix.h>
#endif

int main(int argc, char* argv[])
{
    int i;

    int world_rank = -1, world_size = -1;
    int node_rank = -1,  node_size = -1;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    size_t shm_size = ( argc>1 ? atoi(argv[1]) : 512*1024*1024 ); 
    if (world_rank==0) printf("%d: shm_size = %ld \n", world_rank, (long)shm_size);

    size_t priv_size = ( argc>1 ? atoi(argv[1]) : 256*1024*1024 ); 
    if (world_rank==0) printf("%d: priv_size = %ld \n", world_rank, (long)priv_size);

    int color = world_rank;
#if defined(__bgp__)
    uint32_t xRank, yRank, zRank, tRank;
    uint32_t xSize, ySize, zSize, tSize;
    MPIX_rank2torus( world_rank, &xRank, &yRank, &zRank, &tRank );
    MPIX_rank2torus( world_size, &xSize, &ySize, &zSize, &tSize );
    color = xRank + xSize*yRank + xSize*ySize*zRank;
#endif

    MPI_Comm MPI_Comm_node;
    MPI_Comm_split(MPI_COMM_WORLD, color, 0, &MPI_Comm_node);

    MPI_Comm_rank(MPI_Comm_node, &node_rank);
    MPI_Comm_size(MPI_Comm_node, &node_size);

    int rc = -1;
    int fd = -1;
    char * ptr = NULL;

    if (node_rank==0)
    {
        fd = shm_open("private", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
        if (fd<0) printf("%d: shm_open failed: %d \n", world_rank, fd);
        else      printf("%d: shm_open succeeded: %d \n", world_rank, fd);
        fflush(stdout);

        rc = ftruncate(fd, shm_size);
        if (rc==0) printf("%d: ftruncate succeeded \n", world_rank );
        else       printf("%d: ftruncate failed \n", world_rank );
        fflush(stdout);

        ptr  = mmap( NULL, shm_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
        if (ptr==NULL) printf("%d: mmap failed \n", world_rank );
        else           printf("%d: mmap succeeded \n", world_rank );
        fflush(stdout);

        if (shm_size<5000)
            for (i=0; i<shm_size; i++) printf("%d: ptr[%d] = %c \n", world_rank, i, ptr[i] );
        else    
            for (i=0; i<shm_size; i+=(shm_size/100) ) printf("%d: ptr[%d] = %c \n", world_rank, i, ptr[i] );
        fflush(stdout);
    
        memset(ptr, 'X', shm_size);
        printf("%d: memset 1 succeeded \n", world_rank );
        fflush(stdout);

        if (shm_size<5000)
            for (i=0; i<shm_size; i++) printf("%d: ptr[%d] = %c \n", world_rank, i, ptr[i] );
        else 
            for (i=0; i<shm_size; i+=(shm_size/100) ) printf("%d: ptr[%d] = %c \n", world_rank, i, ptr[i] );
        fflush(stdout);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    char * local = malloc(priv_size);
    if (local==NULL) printf("%d: malloc failed \n", world_rank );
    else           printf("%d: malloc succeeded \n", world_rank );
    fflush(stdout);

    memset(local, 'Y', priv_size);
    printf("%d: memset 2 succeeded \n", world_rank );
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);

    if (node_rank==0)
    {
        if (fd>=0)
        {
            rc = ftruncate(fd, 0);
            if (rc==0) printf("%d: ftruncate succeeded \n", world_rank );
            else       printf("%d: ftruncate failed \n", world_rank );
            fflush(stdout);

            rc = shm_unlink("/bar");
            if (rc==0) printf("%d: shm_unlink succeeded \n", world_rank );
            else       printf("%d: shm_unlink failed \n", world_rank );
            fflush(stdout);
        }

        if (ptr!=NULL)
        {
            rc = munmap(ptr, shm_size);
            if (rc==0) printf("%d: munmap succeeded \n", world_rank );
            else       printf("%d: munmap failed \n", world_rank );
            fflush(stdout);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
