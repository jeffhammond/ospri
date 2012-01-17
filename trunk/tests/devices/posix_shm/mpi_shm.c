#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* BGP only? */
#include <asm-generic/errno-base.h>

#include <pthread.h>
#include <mpi.h>

#define DEV_SHM
//#define POSIX_SHM

#ifdef __bgp__
#  include </bgsys/drivers/ppcfloor/arch/include/spi/kernel_interface.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality_inlines.h>
#endif

#ifdef __bgq__
//#  include <firmware/include/personality.h>
//#  include <spi/include/kernel/process.h>
#  include <spi/include/kernel/location.h>
#endif

int main(int argc, char* argv[])
{
    int i,j;

    int world_rank = -1, num_procs = -1;
    int mpi_result = MPI_SUCCESS;

    int rank_in_node = -1;
    int ranks_per_node = 0;
    int num_nodes = 0;
    int my_node = -1;

    int color = -1, key = -1;
    MPI_Comm IntraNodeComm;

    int node_shmem_bytes; 

    MPI_Init(&argc,&argv);
    mpi_result = MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    assert(mpi_result==MPI_SUCCESS);
    mpi_result = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0)
    {
        char * env_char;
        int units = 1;
        int num_count = 0;
        env_char = getenv("NODE_SHARED_MEMORY");
        if (env_char!=NULL)
        {
            if      ( NULL != strstr(env_char,"G") ) units = 1000000000;
            else if ( NULL != strstr(env_char,"M") ) units = 1000000;
            else if ( NULL != strstr(env_char,"K") ) units = 1000;
            else                                     units = 1;

            num_count = strspn(env_char, "0123456789");
            memset( &env_char[num_count], ' ', strlen(env_char)-num_count);

            node_shmem_bytes = units * atoi(env_char);
            printf("%7d: NODE_SHARED_MEMORY = %d bytes \n", world_rank, node_shmem_bytes );
        }
        else
        {
            node_shmem_bytes = getpagesize();
            printf("%7d: NODE_SHARED_MEMORY = %d bytes \n", world_rank, node_shmem_bytes );
        }
    }
    mpi_result = MPI_Bcast( &node_shmem_bytes, 1, MPI_INT, 0, MPI_COMM_WORLD );
    assert(mpi_result==MPI_SUCCESS);

    int node_shmem_count = node_shmem_bytes/sizeof(double);

    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

#if defined(__bgq__)
    rank_in_node = Kernel_MyTcoord();
#elif defined(__bgp__)
    rank_in_node = Kernel_PhysicalProcessorID();
#else
    rank_in_node = world_rank;
#endif

    /* int MPI_Allreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) */
    mpi_result = MPI_Allreduce( &rank_in_node, &ranks_per_node, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);
    ranks_per_node++; /* change from [0,n-1] to [1,n] */

    num_nodes = num_procs/ranks_per_node;
    assert( (num_procs % ranks_per_node)==0 );

    my_node = (world_rank - rank_in_node)/ranks_per_node;

    printf("%7d: rank_in_node = %2d, ranks_per_node = %2d, my_node = %5d, num_nodes = %5d, world_rank = %7d, num_procs = %7d \n",
            world_rank, rank_in_node, ranks_per_node, my_node, num_nodes, world_rank, num_procs);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    color = my_node;
    key   = rank_in_node;

    /* int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm) */
    mpi_result = MPI_Comm_split(MPI_COMM_WORLD, color, key, &IntraNodeComm);
    assert(mpi_result==MPI_SUCCESS);

    int subcomm_rank = -1;
    mpi_result = MPI_Comm_rank(IntraNodeComm, &subcomm_rank);
    assert(mpi_result==MPI_SUCCESS);

#if defined(POSIX_SHM)
    int fd = shm_open("/foo", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) printf("%7d: shm_open failed: %d \n", world_rank, fd);
    else      printf("%7d: shm_open succeeded: %d \n", world_rank, fd);
#elif defined(DEV_SHM)
    int fd = open("/dev/shm/foo", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
    if (fd<0) printf("%7d: open failed: %d \n", world_rank, fd);
    else      printf("%7d: open succeeded: %d \n", world_rank, fd);
#else
    int fd = -1;
    printf("%7d: no file backing \n", world_rank);
#endif

    if (fd>=0 && subcomm_rank==0)
    {
        int rc = ftruncate(fd, node_shmem_bytes);
        if (rc==0) printf("%7d: ftruncate succeeded \n", world_rank);
        else       printf("%7d: ftruncate failed \n", world_rank);
    }

    double * ptr = mmap( NULL, node_shmem_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (ptr==NULL) printf("%7d: mmap failed \n", world_rank);
    else           printf("%7d: mmap succeeded \n", world_rank);

    fflush(stdout);
    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    for (i=0; i<ranks_per_node; i++)
    {
        if (i==subcomm_rank)
       {
            printf("%7d: subcomm_rank %d setting the buffer \n", world_rank, subcomm_rank );
            for (j=0; j<node_shmem_count; j++ ) ptr[j] = (double)i;
            printf("%7d: memset succeeded \n", world_rank);

            int rc = msync(ptr, node_shmem_bytes, MS_INVALIDATE | MS_SYNC);
            if (rc==0) printf("%7d: msync succeeded \n", world_rank);
            else       printf("%7d: msync failed \n", world_rank);
        }

        fflush(stdout);
        mpi_result = MPI_Barrier(MPI_COMM_WORLD);
        assert(mpi_result==MPI_SUCCESS);

        printf("%7d: ptr = %lf ... %lf \n", world_rank, ptr[0], ptr[node_shmem_count-1]);
        fflush(stdout);

        mpi_result = MPI_Barrier(MPI_COMM_WORLD);
        assert(mpi_result==MPI_SUCCESS);
    }

#if defined(POSIX_SHM)
    if (fd>=0)
    {
        int rc = shm_unlink("/foo");
        if (rc==0) printf("%7d: shm_unlink succeeded \n", world_rank);
        else       printf("%7d: shm_unlink failed \n", world_rank);
    }
#elif defined(DEV_SHM)
    if (fd>=0 && subcomm_rank==0)
    {
        int rc = -1;

        rc = ftruncate(fd, 0);
        if (rc==0) printf("%7d: ftruncate succeeded \n", world_rank);
        else       printf("%7d: ftruncate failed \n", world_rank);

        rc = close(fd);
        if (rc==0) printf("%7d: close succeeded \n", world_rank);
        else       printf("%7d: close failed \n", world_rank);
    }
#endif

    if (ptr!=NULL)
    {
        int rc = munmap(ptr, node_shmem_bytes);
        if (rc==0) printf("%7d: munmap succeeded \n", world_rank);
        else       printf("%7d: munmap failed \n", world_rank);
    }

    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0) printf("%7d: all done! \n", world_rank );

    MPI_Finalize();

    return 0;
}
