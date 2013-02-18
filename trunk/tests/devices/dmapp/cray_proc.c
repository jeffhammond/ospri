#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pmi.h>
#include <pmi2.h>
#include <rca_lib.h>
#include <mpi.h>

int main(int argc, char * argv[])
{
  int rc;
  int rank, size;

  MPI_Init(&argc, &argv);

  int verbose = ( argc > 1 ? atoi(argv[1]) : 0 );

  FILE * pFile;
  pFile = fopen ("myfile.txt","w");
  if (pFile!=NULL)
  {
    fputs ("fopen example",pFile);
    fclose (pFile);
  }



  fflush(stdout);

  MPI_Finalize();

  return 0;
}
