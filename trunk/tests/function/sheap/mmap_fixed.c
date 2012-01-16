#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int main(int argc, char* argv[])
{
    int world_rank = -1, num_procs = -1;
    int mpi_result = MPI_SUCCESS;

    int rank_in_node = -1;
    int ranks_per_node = 0;
    int num_nodes = 0;
    int my_node = -1;

    int color = -1, key = -1;
    MPI_Comm NodeRankComm;

    int sheap_size = 10*1000*1000; /* 10 MB by default */

    void * ptr  = NULL;
    void * addr = NULL;

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
        env_char = getenv("BG_SYMMETRIC_HEAP");
        if (env_char!=NULL) 
        {
            if      ( NULL != strstr(env_char,"G") ) units = 1000000000;
            else if ( NULL != strstr(env_char,"M") ) units = 1000000;
            else if ( NULL != strstr(env_char,"K") ) units = 1000;
            else                                     units = 1;

            num_count = strspn(env_char, "0123456789");
            memset( &env_char[num_count], ' ', strlen(env_char)-num_count);
        }
        sheap_size = units * atoi(env_char);
        printf("%7d: BG_SYMMETRIC_HEAP = %d bytes \n", world_rank, sheap_size );
    }
    mpi_result = MPI_Bcast( &sheap_size, 1, MPI_INT, 0, MPI_COMM_WORLD );
    assert(mpi_result==MPI_SUCCESS);

    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

#if defined(__bgq__)
    rank_in_node = Kernel_MyTcoord();
#elif defined(__bgp__)
    rank_in_node = Kernel_PhysicalProcessorID();
#else
    rank_in_node = world_rank%2;
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

    color = rank_in_node;
    key   = my_node;

    /* int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm) */
    mpi_result = MPI_Comm_split(MPI_COMM_WORLD, color, key, &NodeRankComm);
    assert(mpi_result==MPI_SUCCESS);

    int subcomm_rank = -1;
    mpi_result = MPI_Comm_rank(NodeRankComm, &subcomm_rank);
    assert(mpi_result==MPI_SUCCESS);

    /* void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset); */
    if (subcomm_rank==0) /* on node 0 */
    {
        ptr = mmap( 0, sheap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 );
        printf("%7d: mmap %d bytes at address %p \n", world_rank, sheap_size, ptr );
        fflush(stdout);
        addr = ptr;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm) */
    mpi_result = MPI_Bcast( &addr, sizeof(void*), MPI_BYTE, 0, NodeRankComm );
    assert(mpi_result==MPI_SUCCESS);

    if (subcomm_rank>0) /* not on node 0 */
    {
        ptr = mmap( addr , sheap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0 );
        //if (addr==ptr) printf("%7d: trying to mmap the same address as the root: wanted %p, got %p (SUCCESS) \n", world_rank, addr, ptr );
        //else           printf("%7d: trying to mmap the same address as the root: wanted %p, got %p (FAILURE) \n", world_rank, addr, ptr );
        printf("%7d: trying to mmap the same address as the root: wanted %p, got %p (%s) \n", world_rank, addr, ptr, ptr==addr ? "SUCCESS" : "FAILURE" );
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank==0) printf("%7d: all done! \n", world_rank );

    MPI_Finalize();

    return 0;
}
