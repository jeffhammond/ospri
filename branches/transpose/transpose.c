#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <mpi.h>

#include "hpm.h"
#include "getticks.h"
#include "safemalloc.h"

#define ALIGNMENT 128

int main(int argc, char* argv[])
{
    int rc;

    int desired = MPI_THREAD_FUNNELED;
    int provided;
    rc = MPI_Init_thread( &argc , &argv , desired , &provided ); assert( rc == MPI_SUCCESS );

    int rank, size;
    rc = MPI_Comm_rank( MPI_COMM_WORLD , &rank ); assert( rc == MPI_SUCCESS );
    rc = MPI_Comm_size( MPI_COMM_WORLD , &size ); assert( rc == MPI_SUCCESS );

    int max = ( argc>1 ? atoi(argv[1]) : 4000 );

    if ( rank == 0 )
    {
        unsigned long long * d0 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d1 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d2 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d3 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d4 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d5 = safemalloc( max * sizeof(unsigned long long) );

        /*           12345678901234567890123456789012 */
        char * n0 = "reference - direct copy";
        char * n1 = "basic w/ stride-1 stores";
        char * n2 = "basic w/ stride-1 loads";
        char * n3 = "pragma unroll 4x4 + s1 stores";
        char * n4 = "pragma unroll 4x4 + s1 loads";
        char * n5 = "";

        for ( int n=2 ; n<max ; n++)
        {
            int N = (n*n);

            double * A;
            double * B;
     
            unsigned long long t0, t1;

            A = safemalloc( N * sizeof(double) );
            B = safemalloc( N * sizeof(double) );
     
            for ( int i=0 ; i<N ; i++ ) A[i] = (double)i;
            for ( int i=0 ; i<N ; i++ ) B[i] = 0.0;
     
            /* reference - direct copy */
            t0 = getticks();        
            for ( int t=0 ; t<16 ; t++ )
            {
                for ( int i=0 ; i<n ; i++ )
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[i*n+j];
            }
            t1 = getticks();        
            d0[n] = (t1-t0)/16;

            /* basic w/ stride-1 stores */
            t0 = getticks();        
            for ( int t=0 ; t<16 ; t++ )
            {
                for ( int i=0 ; i<n ; i++ )
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d1[n] = (t1-t0)/16;
     
            /* basic w/ stride-1 loads */
            t0 = getticks();        
            for ( int t=0 ; t<16 ; t++ )
            {
                for ( int j=0 ; j<n ; j++ )
                    for ( int i=0 ; i<n ; i++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d2[n] = (t1-t0)/16;
     
            /* pragma unroll 4x4 + s1 stores */
            t0 = getticks();        
            for ( int t=0 ; t<16 ; t++ )
            {
                #pragma unroll(4)
                for ( int i=0 ; i<n ; i++ )
                    #pragma unroll(4)
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d3[n] = (t1-t0)/16;
     
            /* pragma unroll 4x4 + s1 loads */
            t0 = getticks();        
            for ( int t=0 ; t<16 ; t++ )
            {
                #pragma unroll(4)
                for ( int j=0 ; j<n ; j++ )
                    #pragma unroll(4)
                    for ( int i=0 ; i<n ; i++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d4[n] = (t1-t0)/16;
     
            free(B);
            free(A);

        }

        /* print analysis */
        printf( "%4s x %4s %32s %32s %32s \n" , "n" , "n" , n0 , n1 , n2 );
        for ( int n=2 ; n<max ; n++)
        {
            printf( "%4d x %4d %32llu %32llu %32llu \n" , n , n , d0[n] , d1[n] , d2[n] );
        }
        fflush( stdout );

        free(d5);
        free(d4);
        free(d3);
        free(d2);
        free(d1);
        free(d0);
    }
 
    rc = MPI_Finalize();
    assert( rc == MPI_SUCCESS );

    return 0;
}
