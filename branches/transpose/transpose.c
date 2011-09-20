#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef MPI
    #include <mpi.h>
#endif

#include "hpm.h"
#include "getticks.h"
#include "safemalloc.h"

#define ALIGNMENT 128
#define REPEAT 16

int main(int argc, char* argv[])
{
    int rank = 0;

#ifdef MPI
    int rc;
    int provided;
    rc = MPI_Init_thread( &argc , &argv , MPI_THREAD_FUNNELED , &provided ); assert( rc == MPI_SUCCESS );
    rc = MPI_Comm_rank( MPI_COMM_WORLD , &rank ); assert( rc == MPI_SUCCESS );
#endif

    int min = ( argc>1 ? atoi(argv[1]) : 2 );
    int max = ( argc>2 ? atoi(argv[2]) : 400 );
    int inc = ( argc>3 ? atoi(argv[3]) : 1 );

    if ( rank == 0 )
    {
        unsigned long long * d0 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d1 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d2 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d3 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d4 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d5 = safemalloc( max * sizeof(unsigned long long) );
        unsigned long long * d6 = safemalloc( max * sizeof(unsigned long long) );

        /*           12345678901234567890123456789012 */
        char * n0 = "reference - direct copy";
        char * n1 = "basic w/ stride-1 stores";
        char * n2 = "basic w/ stride-1 loads";
        char * n3 = "pragma unroll 4x4 + s1 stores";
        char * n4 = "pragma unroll 4x4 + s1 loads";
        char * n5 = "manual unroll 4x4 + s1 stores";
        char * n6 = "manual unroll 4x4 + s1 loads";

        printf( "starting test... \n" );
        fprintf( stderr , "%4s %32s %32s %32s %32s %32s \n" , "n" , n0 , n1 , n2 , n3 , n4);
        fflush( stderr );

        for ( int n=min ; n<max ; n+=inc )
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
            for ( int t=0 ; t<REPEAT ; t++ )
            {
                for ( int i=0 ; i<n ; i++ )
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[i*n+j];
            }
            t1 = getticks();        
            d0[n] = (t1-t0)/REPEAT;

            /* basic w/ stride-1 stores */
            t0 = getticks();        
            for ( int t=0 ; t<REPEAT ; t++ )
            {
                for ( int i=0 ; i<n ; i++ )
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d1[n] = (t1-t0)/REPEAT;
     
            /* basic w/ stride-1 loads */
            t0 = getticks();        
            for ( int t=0 ; t<REPEAT ; t++ )
            {
                for ( int j=0 ; j<n ; j++ )
                    for ( int i=0 ; i<n ; i++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d2[n] = (t1-t0)/REPEAT;
     
            /* pragma unroll 4x4 + s1 stores */
            t0 = getticks();        
            for ( int t=0 ; t<REPEAT ; t++ )
            {
                #pragma unroll(4)
                for ( int i=0 ; i<n ; i++ )
                    #pragma unroll(4)
                    for ( int j=0 ; j<n ; j++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d3[n] = (t1-t0)/REPEAT;
     
            /* pragma unroll 4x4 + s1 loads */
            t0 = getticks();        
            for ( int t=0 ; t<REPEAT ; t++ )
            {
                #pragma unroll(4)
                for ( int j=0 ; j<n ; j++ )
                    #pragma unroll(4)
                    for ( int i=0 ; i<n ; i++ )
                        B[i*n+j] = A[j*n+i];
            }
            t1 = getticks();        
            d4[n] = (t1-t0)/REPEAT;

            if ( n<7 )
            {
                printf( "A: \n" );
                for ( int i=0 ; i<n ; i++ )
                {
                    for ( int j=0 ; j<n ; j++ ) printf( "%12.1lf" , A[i*n+j]);
                    printf( "\n" );
                }

                printf( "B: \n" );
                for ( int i=0 ; i<n ; i++ )
                {
                    for ( int j=0 ; j<n ; j++ ) printf( "%12.1lf" , B[i*n+j]);
                    printf( "\n" );
                }
            }

            /* this is just for the neurotic person who cannot wait until the end for data */
            fprintf( stderr , "%4d %32llu %32llu %32llu %32llu %32llu \n" , n , d0[n] , d1[n] , d2[n] , d3[n] , d4[n] );
 
            free(B);
            free(A);
        }
        fflush( stderr );

        /* print analysis */
        printf( "timing in cycles \n" );
        printf( "%4s %32s %32s %32s %32s %32s \n" , "n" , n0 , n1 , n2 , n3 , n4);
        for ( int n=min ; n<max ; n+=inc)
            printf( "%4d %32llu %32llu %32llu %32llu %32llu \n" , n , d0[n] , d1[n] , d2[n] , d3[n] , d4[n] );

        printf( "ratio relative to direct copy \n" );
        printf( "%4s %32s %32s %32s %32s %32s \n" , "n" , n0 , n1 , n2 , n3 , n4);
        for ( int n=min ; n<max ; n+=inc)
        {
            double c = 1.0 / d0[n];
            printf( "%4d %32lf %32lf %32lf %32lf %32lf \n" , n , 1.0 , c*d1[n] , c*d2[n] , c*d3[n] , c*d4[n] );
        }

        printf( "...the end \n" );
        fflush( stdout );

        free(d6);
        free(d5);
        free(d4);
        free(d3);
        free(d2);
        free(d1);
        free(d0);
    }
 
#ifdef MPI
    rc = MPI_Finalize();
    assert( rc == MPI_SUCCESS );
#endif

    return 0;
}
