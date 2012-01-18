#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

#define NONPORTABLE
//#define DEBUG

#if defined(__bgp__) && defined(NONPORTABLE)
#  include </bgsys/drivers/ppcfloor/arch/include/spi/kernel_interface.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality.h>
#  include </bgsys/drivers/ppcfloor/arch/include/common/bgp_personality_inlines.h>
#endif

#if defined(__bgq__) && defined(NONPORTABLE)
//#  include <firmware/include/personality.h>
//#  include <spi/include/kernel/process.h>
#  include <spi/include/kernel/location.h>
#endif

static inline int xstrcmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
} 

int main(int argc, char* argv[])
{
    int mpi_result = MPI_SUCCESS;

    mpi_result = MPI_Init(&argc,&argv);
    assert(mpi_result==MPI_SUCCESS);

    int world_rank = -1, world_size = -1;

    mpi_result = MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    assert(mpi_result==MPI_SUCCESS);
    mpi_result = MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0) printf("world_rank = %d, world_size = %d \n", world_rank, world_size);
    fflush(stdout);
    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    MPI_Comm NodeComm;

#if MPI_VERSION >= 3
    mpi_result = MPI_Comm_split_type(MPI_COMM_WORLD, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
    assert(mpi_result==MPI_SUCCESS);
#elif defined(MPICH2) && (MPICH2_NUMVERSION>10500000)
    mpi_result = MPIX_Comm_split_type(MPI_COMM_WORLD, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
    assert(mpi_result==MPI_SUCCESS);
#else
    int color = world_rank;
# if defined(__bgq__) || defined(__bgp__) && defined(NONPORTABLE)
#  warning nonportable
    int ranks_per_node = 0;
    int rank_in_node = -1;
#  if defined(__bgq__)
    rank_in_node = Kernel_MyTcoord();
#  elif defined(__bgp__)
    rank_in_node = Kernel_PhysicalProcessorID();
#  endif
    mpi_result = MPI_Allreduce( &rank_in_node, &ranks_per_node, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);
    ranks_per_node++; /* change from [0,n-1] to [1,n] */
    int my_node = (world_rank - rank_in_node)/ranks_per_node;

#  ifdef DEBUG
    int num_nodes = world_size/ranks_per_node;
    printf("%7d: rank_in_node = %2d, ranks_per_node = %2d, my_node = %5d, num_nodes = %5d \n",
            world_rank, rank_in_node, ranks_per_node, my_node, num_nodes);
    fflush(stdout);
#  endif
    color = my_node;
# else /* not Blue Gene */
    int namelen = 0;
    char procname[MPI_MAX_PROCESSOR_NAME];

    memset(procname, '\0', MPI_MAX_PROCESSOR_NAME);

    MPI_Get_processor_name( procname, &namelen );
    printf("%d: processor name = %s gethostid = %ld \n" , world_rank, procname, gethostid() );
    fflush(stdout);

    int max_namelen = -1;
    mpi_result = MPI_Allreduce( &namelen, &max_namelen, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);
    assert(max_namelen>0);

    char ** procname_array   = NULL;
    char *  procname_storage = NULL;
    int  *  procname_colors  = NULL;

    if (world_rank==0)
    {
        procname_array = malloc( world_size * sizeof(char *) );
        assert( procname_array != NULL );

        procname_storage = malloc( world_size * max_namelen * sizeof(char) );
        assert( procname_storage != NULL );

        memset( procname_storage, '\0', world_size * max_namelen);

        int i;
        for (i=0; i<world_size; i++)
            procname_array[i] = &procname_storage[i*max_namelen];
    }

    mpi_result = MPI_Gather( procname, max_namelen, MPI_CHAR, procname_storage, max_namelen, MPI_CHAR, 0, MPI_COMM_WORLD );
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0)
    {
        int i;
#  ifdef DEBUG
        for (i=0; i<world_size; i++)
        {
            printf("node %d is ", i);
            int j;
            for (j=0; j<max_namelen; j++) printf("%c", procname_array[i][j] );
            printf(" in procname_array\n");
        }
        fflush(stdout);
#  endif
        qsort(procname_array, world_size, sizeof(char *), (void*) &xstrcmp);

        procname_colors = malloc( world_size * sizeof(int) );
        assert( procname_colors != NULL );

        int color = 0;
        procname_colors[0] = color;
        for (i=1; i<world_size; i++)
        {
            if (0!=strncmp(procname_array[i], procname_array[i-1], max_namelen)) color++;
            procname_colors[i] = color;
#  ifdef DEBUG
            printf("procname_array[%d] = ", i);
            int j;
            for (j=0; j<max_namelen; j++) printf("%c", procname_array[i][j] );
            printf(" \n");

            printf("strcmp(procname_array[i], procname_array[i-1])) = %d \n", strncmp( procname_array[i], procname_array[i-1], max_namelen ) );
#  endif
        }

#  ifdef DEBUG
        for (i=0; i<world_size; i++)
        {
            printf("node %d is ", i);
            int j;
            for (j=0; j<max_namelen; j++) printf("%c", procname_array[i][j] );
            printf(" in procname_array \n");

            printf("node %d color is %d \n", i, procname_colors[i] );
        }
        fflush(stdout);
#  endif

        free(procname_storage);
        free(procname_array);
    }
    mpi_result = MPI_Scatter( procname_colors, 1, MPI_INT, &color, 1, MPI_INT, 0, MPI_COMM_WORLD );
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0) 
        free(procname_colors);

# endif /* if BG else MPI_Get_proc_name */
    mpi_result = MPI_Comm_split(MPI_COMM_WORLD, color, 0, &NodeComm);
    assert(mpi_result==MPI_SUCCESS);
#endif /* MPIX_Comm_split_type */

    int subcomm_rank = -1;
    mpi_result = MPI_Comm_rank(NodeComm, &subcomm_rank);
    assert(mpi_result==MPI_SUCCESS);

    printf("world_rank %d is subcomm_rank %d \n", world_rank, subcomm_rank);

    mpi_result = MPI_Barrier(MPI_COMM_WORLD);
    assert(mpi_result==MPI_SUCCESS);

    if (world_rank==0) printf("%7d: all done! \n", world_rank );

    MPI_Finalize();

    return 0;
}
