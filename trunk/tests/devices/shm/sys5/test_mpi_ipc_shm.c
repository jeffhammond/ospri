#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* BGP only? */
#include <asm-generic/errno-base.h>

#include <pthread.h>
#include <mpi.h>

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

#if (MPI_VERSION >= 3) || defined(MPICH2) && (MPICH2_NUMVERSION>10500000)
#else
int OSPU_Comm_split_node(MPI_Comm oldcomm, MPI_Comm * newcomm);
#endif

int main(int argc, char* argv[])
{
    int i,j;

    int world_rank = -1, num_procs = -1;
    int mpi_result = MPI_SUCCESS;

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

#if MPI_VERSION >= 3
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &IntraNodeComm);
#elif defined(MPICH2) && (MPICH2_NUMVERSION>10500000)
    MPIX_Comm_split_type(MPI_COMM_WORLD, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &IntraNodeComm);
#else
#warning need to link against src/devices/common/ospu_mpi_comm_node.c
    OSPU_Comm_split_node(MPI_COMM_WORLD, &IntraNodeComm);
#endif

    int node_rank = -1;
    mpi_result = MPI_Comm_rank(IntraNodeComm, &node_rank);
    assert(mpi_result==MPI_SUCCESS);

    int node_size = 0;
    mpi_result = MPI_Comm_size(MPI_COMM_WORLD, &node_size);
    assert(mpi_result==MPI_SUCCESS);

    /* setup shm */

    int shmtag;
    if (0==node_rank)
    {
        shmtag = shmget(IPC_PRIVATE, node_shmem_bytes, 0666);
        if (shmtag<0) printf("shmget failed: %d \n", shmtag);
        else          printf("shmget succeeded: %d \n", shmtag);
    }
    mpi_result = MPI_Bcast( &shmtag, 1, MPI_INT, 0, IntraNodeComm);
    assert(mpi_result==MPI_SUCCESS);

    double * ptr = shmat(shmtag, NULL, 0);
    if (ptr==NULL) printf("shmat failed: %d \n", shmtag);
    else           printf("shmat succeeded: %d \n", shmtag);

    fflush(stdout);
    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    for (i=0; i<node_size; i++)
    {
        if (i==node_rank)
       {
            printf("%7d: node_rank %d setting the buffer \n", world_rank, node_rank );
            for (j=0; j<node_shmem_count; j++ ) ptr[j] = (double)world_rank;
            printf("%7d: memset succeeded \n", world_rank);

            /* sync shm */
        }

        fflush(stdout);
        mpi_result = MPI_Barrier(MPI_COMM_WORLD);
        assert(mpi_result==MPI_SUCCESS);

        printf("%7d: ptr = %lf ... %lf \n", world_rank, ptr[0], ptr[node_shmem_count-1]);
        fflush(stdout);

        mpi_result = MPI_Barrier(MPI_COMM_WORLD);
        assert(mpi_result==MPI_SUCCESS);
    }

    /* delete shm */
    int rc = shmdt(ptr);
    if (rc==0) printf("shmdt succeeded: %d \n", rc);
    else       printf("shmdt failed: %d \n", rc);

    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0) printf("%7d: all done! \n", world_rank );
    fflush(stdout);

    MPI_Finalize();

    return 0;
}
