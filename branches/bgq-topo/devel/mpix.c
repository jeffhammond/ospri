#include <stdio.h>

#include <mpi.h>
#include <mpix.h>

#if 0
 typedef struct
 {
/* These fields will be used on all platforms. */
   unsigned prank;    /**< Physical rank of the node (irrespective of mapping) */
   unsigned psize;    /**< Size of the partition (irrespective of mapping) */
   unsigned ppn;      /**< Processes per node ("T+P" size) */
   unsigned coreID;   /**< Core+Thread info. Value ranges from 0..63 */

   unsigned clockMHz; /**< Frequency in MegaHertz */
   unsigned memSize;  /**< Size of the core memory in MB */

/* These fields are only set on torus platforms (i.e. Blue Gene) */
   unsigned torus_dimension;              /**< Actual dimension for the torus */
   unsigned Size[MPIX_TORUS_MAX_DIMS];    /**< Max coordinates on the torus */
   unsigned Coords[MPIX_TORUS_MAX_DIMS];  /**< This node's coordinates */
   unsigned isTorus[MPIX_TORUS_MAX_DIMS]; /**< Do we have wraparound links? */

/* These fields are only set on systems using Blue Gene IO psets. */
   unsigned rankInPset;
   unsigned sizeOfPset;
   unsigned idOfPset;
 } MPIX_Hardware_t;
#endif

int main(int argc, char* argv[])
{
  int rank;
  int namelen;
  char procname[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Get_processor_name( procname, &namelen );

  MPIX_Hardware_t hw;
  MPIX_Hardware(&hw);

  printf("%d: processor name = %s, Size = {%2d,%2d,%2d,%2d,%1d}, Coords = {%2d,%2d,%2d,%2d,%1d}, isTorus = {%1d,%1d,%1d,%1d,%1d} \n",
         rank, procname,
         hw.Size[0], hw.Size[1], hw.Size[2], hw.Size[3], hw.Size[4],
         hw.Coords[0], hw.Coords[1], hw.Coords[2], hw.Coords[3], hw.Coords[4],
         hw.isTorus[0], hw.isTorus[1], hw.isTorus[2], hw.isTorus[3], hw.isTorus[4]);

  fflush(stdout);

  MPI_Finalize();

  return 0;
}
