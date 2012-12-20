#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

#define DEBUG

static inline int xstrcmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
} 

int MPE_Comm_split_node(MPI_Comm * NodeComm)
{
    int world_rank = -1, world_size = -1;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int namelen = 0;
    char procname[MPI_MAX_PROCESSOR_NAME];
    memset(procname, '\0', MPI_MAX_PROCESSOR_NAME);
    MPI_Get_processor_name( procname, &namelen );
#  ifdef DEBUG
    printf("%d: processor name = %s gethostid = %ld \n" , world_rank, procname, gethostid() );
    fflush(stdout);
#  endif

    int max_namelen = -1;
    MPI_Allreduce( &namelen, &max_namelen, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
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

    MPI_Gather( procname, max_namelen, MPI_CHAR, procname_storage, max_namelen, MPI_CHAR, 0, MPI_COMM_WORLD );

    int color = world_rank;

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

        color = 0;
        procname_colors[0] = color;
        for (i=1; i<world_size; i++)
        {
            if (0!=strncmp(procname_array[i], procname_array[i-1], max_namelen)) color++;
            procname_colors[i] = color;
#  ifdef DEBUG
            printf("procname_array[%d] = ", i );
            int j;
            for (j=0; j<max_namelen; j++) printf("%c", procname_array[i][j] );
            printf(" \n");
            printf("strcmp(procname_array[%d], procname_array[%d])) = %d \n", i, i-1, strncmp( procname_array[i], procname_array[i-1], max_namelen ) );
            printf("procname_colors[%d] = %d \n", i, procname_colors[i] );
#  endif
        }

        free(procname_storage);
        free(procname_array);
    }
    MPI_Scatter( procname_colors, 1, MPI_INT, &color, 1, MPI_INT, 0, MPI_COMM_WORLD );

    if (world_rank==0) 
        free(procname_colors);

    MPI_Comm_split(MPI_COMM_WORLD, color, 0, NodeComm);

    return MPI_SUCCESS;
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc,&argv);

    int world_rank = -1, world_size = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Comm NodeComm;
#if MPI_VERSION >= 3
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
#elif defined (MPICH2) && (MPICH2_NUM_VERSION >= MPICH2_CALC_VERSION(1,5,0,0,0)) && 0
    MPIX_Comm_split_type(MPI_COMM_WORLD, MPIX_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &NodeComm);
#else
    MPE_Comm_split_node(&NodeComm);
#endif

    int node_rank = -1,  node_size = -1;
    MPI_Comm_rank(NodeComm, &node_rank);
    MPI_Comm_size(NodeComm, &node_size);

    int namelen = 0;
    char procname[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name( procname, &namelen );

    printf("%s: %d of %d on node, %d of %d on world \n", procname, node_rank, node_size, world_rank, world_size );

    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank==0) printf("%7d: all done! \n", world_rank );
    fflush(stdout);

    MPI_Finalize();

    return 0;
}
