#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>
#include "mympix.h"

#include "safemalloc.h"

#define DEBUG YES

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

#if defined(USE_MPI_INIT)

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

#else

	int provided = -1, requested = -1;

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

	if (provided>requested && world_rank==0)
		printf("MPI_Init_thread returned %s instead of %s, but this is okay. \n",
				MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
	if (provided<requested && world_rank==0)
	{
		printf("MPI_Init_thread returned %s instead of %s so the test will exit. \n",
				MPI_THREAD_STRING(provided), MPI_THREAD_STRING(requested) );
		exit(1);
	}

#endif

	MPI_Comm_size( MPI_COMM_WORLD, &world_size );
	if (world_rank==0) printf("MPI test program running on %d ranks. \n", world_size);

	/*********************************************************************************
	 *                            CREATE WINDOWS
	 *********************************************************************************/

	int max = (argc>1 ? atoi(argv[1]) : 1024*1024);

	int * wbuf = NULL;
	MPI_Win window;
	MPIX_Win_allocate(max * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, (void*) wbuf, &window);

	int * lbuf = NULL;
	MPI_Alloc_mem(max * sizeof(int), MPI_INFO_NULL, &lbuf);

	/*********************************************************************************
	 *                            TEST PUT
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                    MPI_Put(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, window);
                    MPI_Win_unlock(target, window);
                }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Put individual from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST PUT
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
            {
                MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Put(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, window);
                }
                MPI_Win_unlock(target, window);
            }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Put bulk from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST GET
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                    MPI_Get(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, window);
                    MPI_Win_unlock(target, window);
                }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Get individual from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST GET
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
            {
                MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Get(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, window);
                }
                MPI_Win_unlock(target, window);
            }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Get bulk from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST ACCUMULATE
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                    MPI_Accumulate(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, MPI_REPLACE, window);
                    MPI_Win_unlock(target, window);
                }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Acc (Replace) individual from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST ACCUMULATE
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
            {
                MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Accumulate(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, MPI_REPLACE, window);
                }
                MPI_Win_unlock(target, window);
            }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Acc (Replace) bulk from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST ACCUMULATE
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                    MPI_Accumulate(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, MPI_SUM, window);
                    MPI_Win_unlock(target, window);
                }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Acc (Sum) individual from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            TEST ACCUMULATE
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

    for (int target = 0; target<world_size; target++)
        for (int count = 1; count<(max/100); count*=2)
        {
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Aint offset = 0;

            double t0 = MPI_Wtime();
            if (world_rank==0)
            {
                MPI_Win_lock(MPI_LOCK_SHARED, target, MPI_MODE_NOCHECK, window);
                for (int n=0; n<100; n++)
                {
                    offset += count;
                    MPI_Accumulate(&lbuf[offset], count, MPI_INT, target, offset, count, MPI_INT, MPI_SUM, window);
                }
                MPI_Win_unlock(target, window);
            }
            double t1 = MPI_Wtime();

            MPI_Barrier(MPI_COMM_WORLD);

            if (world_rank==0)
                printf("Acc (Sum) bulk from rank %d to rank %d of %d bytes took %lf seconds: %lf MB/s \n", 
                        world_rank, target, count, t1-t0, 100*1e-6*count/(t1-t0) );
        }

	/*********************************************************************************
	 *                            CLEAN UP AND FINALIZE
	 *********************************************************************************/

    if (world_rank==0)
        printf("================================================================= \n");

	MPI_Free_mem((void*)lbuf);

	MPI_Win_free(&window);
	MPI_Free_mem((void*)wbuf);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
