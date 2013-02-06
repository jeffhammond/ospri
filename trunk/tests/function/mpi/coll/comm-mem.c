#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

#include "safemalloc.h"
#include "driver.h"

#define MPI_THREAD_STRING(level)  \
        ( level==MPI_THREAD_SERIALIZED ? "THREAD_SERIALIZED" : \
                ( level==MPI_THREAD_MULTIPLE ? "THREAD_MULTIPLE" : \
                        ( level==MPI_THREAD_FUNNELED ? "THREAD_FUNNELED" : \
                                ( level==MPI_THREAD_SINGLE ? "THREAD_SINGLE" : "WTF" ) ) ) )

int main(int argc, char *argv[])
{
    /*********************************************************************************
     *                            INITIALIZE MPI
     *********************************************************************************/

    int world_size = 0, world_rank = -1;
    int provided = -1;

#if defined(USE_MPI_INIT)

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

#else

    int requested = -1;

#  if defined(USE_MPI_INIT_THREAD_MULTIPLE)
    requested = MPI_THREAD_MULTIPLE;
#  elif defined(USE_MPI_INIT_THREAD_SERIALIZED)
    requested = MPI_THREAD_SERIALIZED;
#  elif defined(USE_MPI_INIT_THREAD_FUNNELED)
    requested = MPI_THREAD_FUNNELED;
#  else
    requested = MPI_THREAD_SINGLE;
#  endif

    MPI_Init_thread( &argc, &argv, requested, &provided );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

    if (world_rank==0)
        print_meminfo(stdout, "after MPI_Init_thread");

    if (provided>requested)
    {
        if (world_rank==0) printf("MPI_Init_thread returned %s instead of %s, but this is okay. \n",
                                  MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
    }
    if (provided<requested)
    {
        if (world_rank==0) printf("MPI_Init_thread returned %s instead of %s so the test will exit. \n",
                                  MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

#endif

    double t0 = MPI_Wtime();

    int is_init = 0;
    MPI_Initialized(&is_init);
    if (world_rank==0) printf("MPI %s initialized. \n", (is_init==1 ? "was" : "was not") );

    MPI_Query_thread(&provided);
    if (world_rank==0) printf("MPI thread support is %s. \n", MPI_THREAD_STRING(provided) );

    MPI_Comm_size( MPI_COMM_WORLD, &world_size );
    if (world_rank==0) printf("MPI test program running on %d ranks. \n", world_size);

    /*********************************************************************************
     *                            SETUP MPI COMMUNICATORS
     *********************************************************************************/

    for (int iter=0; iter<10000; iter++)
    {
        MPI_Barrier( MPI_COMM_WORLD );

        if (world_rank==0)
            printf("iteration %d \n", iter);

        if (world_rank==0)
            print_meminfo(stdout, "before MPI communicator creation");

        if (world_rank==0) printf("MPI_Comm_dup of MPI_COMM_WORLD \n");
        MPI_Comm comm_world_dup;
        MPI_Comm_dup(MPI_COMM_WORLD, &comm_world_dup);

        if (world_rank==0) printf("MPI_Barrier on comm_world_dup \n");
        MPI_Barrier( comm_world_dup );

        if (world_rank==0) printf("MPI_Comm_split of MPI_COMM_WORLD into world_reordered \n");
        MPI_Comm comm_world_reordered;
        MPI_Comm_split(MPI_COMM_WORLD, 0, world_size-world_rank, &comm_world_reordered);

        if (world_rank==0) printf("MPI_Comm_split of MPI_COMM_WORLD into left-right \n");
        MPI_Comm comm_world_leftright;
        int leftright = (world_rank<(world_size/2));
        MPI_Comm_split(MPI_COMM_WORLD, leftright, world_rank, &comm_world_leftright);

        if (world_rank==0) printf("MPI_Barrier on comm_world_leftright \n");
        MPI_Barrier( comm_world_leftright );

        if (world_rank==0) printf("MPI_Comm_split of MPI_COMM_WORLD into odd-even \n");
        MPI_Comm comm_world_oddeven;
        int oddeven = (world_rank%2);
        MPI_Comm_split(MPI_COMM_WORLD, oddeven, world_rank, &comm_world_oddeven);

        if (world_rank==0) printf("MPI_Barrier on comm_world_oddeven \n");
        MPI_Barrier( comm_world_oddeven );

        if (world_rank==0) printf("MPI_Comm_split MPI_COMM_WORLD into (world-1) \n");
        MPI_Comm comm_world_minus_one;
        int left_out = world_rank==(world_size/2);
        MPI_Comm_split(MPI_COMM_WORLD, left_out, world_rank, &comm_world_minus_one);

        if (world_rank==0) printf("MPI_Barrier on comm_world_minus_one \n");
        MPI_Barrier( comm_world_minus_one );

        if (world_rank==0) printf("MPI_Comm_group of group_world from MPI_COMM_WORLD \n");
        MPI_Group group_world;
        MPI_Comm_group(MPI_COMM_WORLD, &group_world);

        int geomprog_size = (world_size==1) ? 1 : ceil(log2(world_size));

        int * geomprog_list = NULL;
        geomprog_list = (int *) safemalloc( geomprog_size * sizeof(int) );

        for (int i=0; i<geomprog_size; i++)
            geomprog_list[i] = pow(2,i)-1;

        if (world_rank==0)
            for (int i=0; i<geomprog_size; i++)
                if (world_rank==0) printf("geomprog_list[%d] = %d \n", i, geomprog_list[i]);

        if (world_rank==0) printf("MPI_Group_incl of group_geomprog (geometric progression) from group_world \n");
        MPI_Group group_geomprog;
        MPI_Group_incl(group_world, geomprog_size, geomprog_list, &group_geomprog);
        MPI_Group_free(&group_world);

        if (world_rank==0) printf("MPI_Comm_create of comm_geomprog from group_geomprog on MPI_COMM_WORLD \n");
        MPI_Comm comm_geomprog;
        MPI_Comm_create(MPI_COMM_WORLD, group_geomprog, &comm_geomprog);
        MPI_Group_free(&group_geomprog);

        if (world_rank==0) printf("MPI_Barrier on comm_geomprog \n");
        for (int i=0; i<geomprog_size; i++)
            if (geomprog_list[i]==world_rank)
                MPI_Barrier( comm_geomprog );

        if (world_rank==0)
            print_meminfo(stdout, "after MPI communicator creation");

        if (comm_geomprog!=MPI_COMM_NULL);
            MPI_Comm_free(&comm_geomprog);

        MPI_Comm_free(&comm_world_minus_one);
        MPI_Comm_free(&comm_world_oddeven);
        MPI_Comm_free(&comm_world_leftright);
        MPI_Comm_free(&comm_world_reordered);
        MPI_Comm_free(&comm_world_dup);

        free(geomprog_list);

        if (world_rank==0)
            print_meminfo(stdout, "after MPI communicator free-ing");

    }

    /*********************************************************************************
     *                            CLEAN UP AND FINALIZE
     *********************************************************************************/

    MPI_Barrier( MPI_COMM_WORLD );

    double t1 = MPI_Wtime();
    double dt = t1-t0;
    if (world_rank==0)
       printf("TEST FINISHED SUCCESSFULLY IN %lf SECONDS \n", dt);
    fflush(stdout);

    if (world_rank==0)
        print_meminfo(stdout, "before MPI_Finalize");

    MPI_Finalize();

    return 0;
}
