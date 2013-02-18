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
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int verbose = ( argc > 1 ? atoi(argv[1]) : 0 );

  FILE * procfile = fopen("/proc/cray_xt/cname","r");
  if (procfile!=NULL) {

    /* format example: c1-0c1s2n1 c3-0c2s15n3 */

    char str[255];
    fscanf(procfile, "%s", str);

    rewind(procfile);
    char a, b, c, d;
    int i, j, k, l, m;
    fscanf(procfile, "%c%d-%d%c%d%c%d%c%d", &a, &i, &j, &b, &k, &c, &l, &d, &m);

    printf("%d: /proc/cray_xt/cname = %s \n", rank, str);
    printf("%d: coords = (%d,%d,%d,%d,%d) \n", rank, i, j, k, l, m);

    fclose(procfile);
  } else {
    fprintf(stderr, "fopen has failed! \n");
    exit(1);
  }

  MPI_Finalize();

  return 0;
}
