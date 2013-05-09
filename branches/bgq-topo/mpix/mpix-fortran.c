/**************************************************************
 *
 * A Fortran-compatible wrapper for Blue Gene/Q MPIX routines
 *
 * Author: Jeff Hammond
 *         Argonne Leadership Computing Facility
 *         jhammond@alcf.anl.gov
 *         February 2013
 *
 * Copyright (c) 2013, UChicago Argonne, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include <mpi.h>
#include <mpix.h>

/* modify this as appropriate for your Fortran compiler */
#define F77_FUNC(lc,UC) lc ## _

#if defined(__cplusplus)
extern "C" {
#endif

static void safecast(MPI_Fint fint, int * cint)
{
    /* if MPI_Fint is 8-bytes, it is theoretically possible for it to overflow,
     * but since the maximum valid value of MPI_Fint arguments is much less than 2^31, 
     * this will only result from incorrect usage. */
    assert( fint < INT_MAX );
    *cint = (int)(fint);
    return;
}

void F77_FUNC(mpix_dump_stacks,MPIX_DUMP_STACKS)(MPI_Fint * ierr)
{
  *ierr = MPI_SUCCESS;
  MPIX_Dump_stacks();
  return;
}

void F77_FUNC(mpix_progress_poke,MPIX_PROGRESS_POKE)(MPI_Fint * ierr)
{
  *ierr = MPI_SUCCESS;
  MPIX_Progress_poke();
  return;
}

void F77_FUNC(mpix_progress_quiesce,MPIX_PROGRESS_QUIESCE)(double * timeout, MPI_Fint * ierr)
{
  *ierr = (MPI_Fint)MPIX_Progress_quiesce(*timeout);
  return;
}

void F77_FUNC(mpix_comm_rank2global,MPIX_COMM_RANK2GLOBAL)(MPI_Fint * comm, MPI_Fint * crank, MPI_Fint * grank, MPI_Fint * ierr)
{
    MPI_Comm mycomm = MPI_Comm_f2c(*comm);
    int mycrank = -1, mygrank = -1;
    safecast(*crank, &mycrank);
    *ierr = (MPI_Fint)MPIX_Comm_rank2global(mycomm, mycrank, &mygrank);
    *grank = (MPI_Fint)mygrank;
    return;
}

void F77_FUNC(mpix_torus_ndims,MPIX_TORUS_NDIMS)(MPI_Fint * numdim, MPI_Fint * ierr)
{
    int mynumdim = -1;
    *ierr = (MPI_Fint)MPIX_Torus_ndims(&mynumdim);
    *numdim = (MPI_Fint)mynumdim;
    return;
}

void F77_FUNC(mpix_rank2torus,MPIX_RANK2TORUS)(MPI_Fint * rank, MPI_Fint * coords, MPI_Fint * ierr)
{
    int torusdim = -1;
    int rc = MPIX_Torus_ndims(&torusdim);
    if (rc!=MPI_SUCCESS) {
        *ierr = (MPI_Fint)rc;
        return;
    }

    int myrank = -1;
    safecast(*rank, &myrank);

    int * mycoords = malloc(torusdim*sizeof(int));
    *ierr = (MPI_Fint)MPIX_Rank2torus(myrank, mycoords);

    for (int i=0; i<torusdim; i++)
        coords[i] = (MPI_Fint)mycoords[i];

    free(mycoords);

    return;
}

void F77_FUNC(mpix_torus2rank,MPIX_TORUS2RANK)(MPI_Fint * coords, MPI_Fint * rank, MPI_Fint * ierr)
{
    int torusdim = -1;
    int rc = MPIX_Torus_ndims(&torusdim);
    if (rc!=MPI_SUCCESS) {
        *ierr = (MPI_Fint)rc;
        return;
    }

    int * mycoords = malloc(torusdim*sizeof(int));
    for (int i=0; i<torusdim; i++)
        safecast(mycoords[i], &(mycoords[i]) );

    int myrank = -1;
    *ierr = (MPI_Fint)MPIX_Torus2rank(mycoords, &myrank);

    *rank = (MPI_Fint)myrank;

    free(mycoords);

    return;
}


void F77_FUNC(mpix_comm_update,MPIX_COMM_UPDATE)(MPI_Fint * comm, MPI_Fint * optimize, MPI_Fint * ierr)
{
    MPI_Comm mycomm = MPI_Comm_f2c(*comm);
    int myoptimize = -1;
    safecast(*optimize, &myoptimize);
    *ierr = (MPI_Fint)MPIX_Comm_update(mycomm, myoptimize);
    return;
}

#if NOT_YET

void F77_FUNC(mpix_get_last_algorithm_name,MPIX_GET_LAST_ALGORITHM_NAME)(MPI_Fint * comm, char * protocol, MPI_Fint * ierr, size_t len)
{
    MPI_Comm mycomm = MPI_Comm_f2c(*comm);

#warning DEAL WITH FORTRAN STRING LENGTH SECRET ARGUMENT PASSING BULLTISH

   *ierr = (MPI_Fint)MPIX_Get_last_algorithm_name(mycomm, protocol, len);
    return;
}

#define MPIX_TORUS_MAX_DIMS 5 /* This is the maximum physical size of the torus */
  typedef struct
  {
/* These fields will be used on all platforms. */
    unsigned prank;    /**< Physical rank of the node (irrespective of mapping) */
    unsigned psize;    /**< Size of the partition (irrespective of mapping) */
    unsigned ppn;      /**< Processes per node */
    unsigned coreID;   /**< Process id; values monotonically increase from 0..63 */

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


  /**
   * \brief Fill in an MPIX_Hardware_t structure
   * \param[in] hw A pointer to an MPIX_Hardware_t structure to be filled in
   */
  int MPIX_Hardware(MPIX_Hardware_t *hw);
#endif


#if defined(__cplusplus)
}
#endif
