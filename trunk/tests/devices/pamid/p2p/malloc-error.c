#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>

void * safemalloc(size_t n) 
{
    void * ptr = malloc( n );
    if ( ptr == NULL )
    {
        fprintf( stderr , "%ld bytes could not be allocated \n" , (long)n );
        exit(n);
    }
    return ptr;
}

static size_t world_size, world_rank = -1;

int main(int argc, char* argv[])
{
  int provided = MPI_THREAD_SINGLE;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided<MPI_THREAD_MULTIPLE) 
    exit(provided);

  /************************************************************************/

  int n = (argc>1 ? atoi(argv[1]) : 1000);

  size_t bytes = 1000 * sizeof(int);
  int *  shared = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    shared[i] = -1;

  int *  local  = (int *) safemalloc(bytes);
  for (int i=0; i<n; i++)
    local[i] = world_rank;

  MPI_Barrier(MPI_COMM_WORLD);

  free(local);
  free(shared);

  /************************************************************************/

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();

  if (world_rank==0)
    printf("%ld: end of test \n", world_rank );
  fflush(stdout);

  return 0;
}

