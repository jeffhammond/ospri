/***************************************************************************

                  COPYRIGHT

The following is a notice of limited availability of the code, and disclaimer
which must be included in the prologue of the code and in all source listings
of the code.

Copyright Notice
 + 2009 University of Chicago

Permission is hereby granted to use, reproduce, prepare derivative works, and
to redistribute to others.  This software was authored by:

Jeff R. Hammond
Leadership Computing Facility
Argonne National Laboratory
Argonne IL 60439 USA
phone: (630) 252-5381
e-mail: jhammond@mcs.anl.gov

                  GOVERNMENT LICENSE

Portions of this material resulted from work developed under a U.S.
Government Contract and are subject to the following license: the Government
is granted for itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable worldwide license in this computer software to reproduce, prepare
derivative works, and perform publicly and display publicly.

                  DISCLAIMER

This computer code material was prepared, in part, as an account of work
sponsored by an agency of the United States Government.  Neither the United
States, nor the University of Chicago, nor any of their employees, makes any
warranty express or implied, or assumes any legal liability or responsibility
for the accuracy, completeness, or usefulness of any information, apparatus,
product, or process disclosed, or represents that its use would not infringe
privately owned rights.

 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int desired = MPI_THREAD_SINGLE;
    int provided;
    MPI_Init_thread(&argc, &argv, desired, &provided);

    int me;
    int nproc;
    MPI_Comm_rank(MPI_COMM_WORLD,&me);
    MPI_Comm_size(MPI_COMM_WORLD,&nproc);

    int max, count;
    max   = ( argc>1 ? atoi(argv[1]) : 1000000 );
    count = ( argc>2 ? atoi(argv[2]) : 100 );
    if (me==0) printf("max = %d bytes count = %d \n", max, count );

    for ( int size=1 ; size<max ; size*=2 )
    {
        int status;

        /* allocate RMA buffers */
        char * winbuf; 
        status = MPI_Alloc_mem( size * sizeof(char), MPI_INFO_NULL, &winbuf );
        assert( status == MPI_SUCCESS && winbuf != NULL );
        memset( winbuf, 'w', size );

        /* register remote pointers */
        MPI_Win win;
        status = MPI_Win_create( winbuf, size * sizeof(char), sizeof(char), MPI_INFO_NULL, MPI_COMM_WORLD, &win );
        assert( status == MPI_SUCCESS );

        char * tmp;
        status = MPI_Alloc_mem( size * sizeof(char), MPI_INFO_NULL, &tmp );
        assert( status == MPI_SUCCESS && tmp != NULL );
        memset( winbuf, 't', size );

        if (me==0)
        {    

            double dt_put=0.0, dt_copy=0.0, dt_get=0.0;
            double t0, t1;

            int mpiassert = 0;

            t0 = MPI_Wtime();
            for ( int r=0 ; r<count ; r++ )
            {
                status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, 0, mpiassert, win );
                assert( status == MPI_SUCCESS );

                status = MPI_Put( tmp, size, MPI_BYTE, 0, 0, size, MPI_BYTE, win );
                assert( status == MPI_SUCCESS );

                status = MPI_Win_unlock( 0, win );
                assert( status == MPI_SUCCESS );
            }
            t1 = MPI_Wtime();
            dt_put = (t1-t0)/count;

            t0 = MPI_Wtime();
            for ( int r=0 ; r<count ; r++ )
            {
                status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, 0, mpiassert, win );
                assert( status == MPI_SUCCESS );

                memcpy( winbuf, tmp, size );

                status = MPI_Win_unlock( 0, win );
                assert( status == MPI_SUCCESS );
            }
            t1 = MPI_Wtime();
            dt_copy = (t1-t0)/count;

            t0 = MPI_Wtime();
            for ( int r=0 ; r<count ; r++ )
            {
                status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, 0, mpiassert, win );
                assert( status == MPI_SUCCESS );

                status = MPI_Get( tmp, size, MPI_BYTE, 0, 0, size, MPI_BYTE, win );
                assert( status == MPI_SUCCESS );

                status = MPI_Win_unlock( 0, win );
                assert( status == MPI_SUCCESS );
            }
            t1 = MPI_Wtime();
            dt_get += (t1-t0)/count;

            printf( "size = %10d time: put = %e s copy = %e s get = %e s BW put = %e B/s copy = %e B/s get = %e B/s \n", 
                     size,          dt_put,    dt_copy,    dt_get,  size/dt_put,  size/dt_copy,  size/dt_get );

        }

        status = MPI_Free_mem(tmp);

        status = MPI_Win_free(&win);
        status = MPI_Free_mem(winbuf);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (me==0) printf("%d: all done\n",me);
    MPI_Finalize();

    return(0);
}



