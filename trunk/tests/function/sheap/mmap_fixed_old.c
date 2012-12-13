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
# ifdef BGQ_SPI_TOPO
#  include <firmware/include/personality.h>
#  include <spi/include/kernel/process.h>
#  include <spi/include/kernel/location.h>
# else
#  include <mpix.h>
# endif
#endif

#if defined(__CRAYXT) || defined(__CRAYXE)
#  include <pmi.h> 
//#  include <rca_lib.h>
#endif

int main(int argc, char* argv[])
{
    int world_rank = -1, world_size = -1;
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
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

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

    MPI_Comm NodeComm;
#if defined(__bgq__)
    int nodeid;
# ifdef BGQ_SPI_TOPO
#  warning Not implemented yet!
    Personality_t pers;

    rc = Kernel_GetPersonality(&pers, sizeof(pers));
    assert(rc==0);

    // this is nonsense...
    nodeid = pers.Network_Config.Acoord;
             pers.Network_Config.Bcoord;
             pers.Network_Config.Ccoord;
             pers.Network_Config.Dcoord;
             pers.Network_Config.Ecoord;
             pers.Network_Config.Anodes;
             pers.Network_Config.Bnodes;
             pers.Network_Config.Cnodes;
             pers.Network_Config.Dnodes;
             pers.Network_Config.Enodes;
# else
    MPIX_Hardware_t hw;
    MPIX_Hardware(&hw);

    nodeid = hw.Size[0] * hw.Coords[1] * hw.Coords[2] * hw.Coords[3] * hw.Coords[4]
           + hw.Size[1] * hw.Coords[2] * hw.Coords[3] * hw.Coords[4]
           + hw.Size[2] * hw.Coords[3] * hw.Coords[4]
           + hw.Size[3] * hw.Coords[4]
           + hw.Size[4];
#endif
    MPI_Comm_split(MPI_COMM_WORLD, nodeid, 0, &NodeComm);
#elif defined(__bgp__)
#  warning There is a better way to do this...
    rank_in_node = Kernel_PhysicalProcessorID();
    MPI_Allreduce( &rank_in_node, &ranks_per_node, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    ranks_per_node++; /* change from [0,n-1] to [1,n] */
#else
#  if defined(__CRAYXT) || defined(__CRAYXE)
    int cray_nid;
#    if defined(__CRAYXT)
    PMI_Portals_get_nid(world_rank, &cray_nid);
#    elif defined(__CRAYXE)
    PMI_Get_nid(world_rank, &cray_nid);
    //PMI_Get_clique_size(&ranks_per_node);
#    endif
    MPI_Comm_split(MPI_COMM_WORLD, cray_nid, 0, &NodeComm);
#  elif MPI_VERSION >= 3
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
#  elif defined(MPICH2) && (MPICH2_NUMVERSION>10500000)
    MPIX_Comm_split_type(MPI_COMM_WORLD, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
#  else
#   error No way to find node communicator!
#  endif
#endif
    MPI_Comm_rank(NodeComm, &rank_in_node);
    MPI_Comm_rank(NodeComm, &ranks_per_node);

    num_nodes = world_size/ranks_per_node;
    assert( (world_size % ranks_per_node)==0 );
    
    my_node = (world_rank - rank_in_node)/ranks_per_node;

    printf("%7d: rank_in_node = %2d, ranks_per_node = %2d, my_node = %5d, num_nodes = %5d, world_rank = %7d, world_size = %7d \n",
            world_rank, rank_in_node, ranks_per_node, my_node, num_nodes, world_rank, world_size);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    color = rank_in_node;
    key   = my_node;

    MPI_Comm_split(MPI_COMM_WORLD, color, key, &NodeRankComm);

    int subcomm_rank = -1;
    MPI_Comm_rank(NodeRankComm, &subcomm_rank);

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
    MPI_Bcast( &addr, sizeof(void*), MPI_BYTE, 0, NodeRankComm );

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
