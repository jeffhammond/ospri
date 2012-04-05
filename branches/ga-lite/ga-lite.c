/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * The following is a notice of limited availability of the code, and disclaimer
 * which must be included in the prologue of the code and in all source listings
 * of the code.
 *
 * Copyright (c) 2010  Argonne Leadership Computing Facility, Argonne National
 * Laboratory
 *
 * Permission is hereby granted to use, reproduce, prepare derivative works, and
 * to redistribute to others.
 *
 *
 *                          LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer listed
 *   in this license in the documentation and/or other materials
 *   provided with the distribution.
 *
 * - Neither the name of the copyright holders nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * The copyright holders provide no reassurances that the source code
 * provided does not infringe any patent, copyright, or any other
 * intellectual property rights of third parties.  The copyright holders
 * disclaim any liability to any recipient for claims brought against
 * recipient by any third party for infringement of that parties
 * intellectual property rights.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  Created on: Sep 20, 2010
 *      Author: Jeff Hammond
 */

#include "ga-lite.h"

/**************************************************************************
 * MACRO DEFINITIONS
 **************************************************************************/

#define GAL_INLINE inline

#define GAL_MALLOC_ALIGNMENT 128

#define GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS 1

#define GAL_DEFAULT_BLOCKSIZE 4096

/**************************************************************************
 * MACRO FUNCTIONS
 **************************************************************************/

#define GALU_ZERO(INOUT,COUNT,DTYPE)                       \
        do {                                                 \
            DTYPE * io = (DTYPE *)INOUT;                       \
            for (int i = 0; i < COUNT; i++) io[i] = (DTYPE)0;  \
        } while (0)

#define GALU_SET(VALUE,INOUT,COUNT,DTYPE)              \
        do {                                             \
            DTYPE * s  = (DTYPE* )VALUE;                   \
            DTYPE * io = (DTYPE *)INOUT;                   \
            for (int i = 0; i < COUNT; i++) io[i] = (*s);  \
        } while (0)

#define GALU_SCALE(SCALING,INOUT,COUNT,DTYPE)          \
        do {                                             \
            DTYPE * s  = (DTYPE *)SCALING;                 \
            DTYPE * io = (DTYPE *)INOUT;                   \
            for (int i = 0; i < COUNT; i++) io[i] *= (*s); \
        } while (0)

#define GALU_ACC(IN,INOUT,COUNT,DTYPE)                   \
        do {                                               \
            DTYPE * in = (DTYPE *)IN;                        \
            DTYPE * io = (DTYPE *)INOUT;                     \
            for (int i = 0; i < COUNT; i++) io[i] += in[i];  \
        } while (0)

#define GALU_SCALE_ACC(SCALING,IN,INOUT,COUNT,DTYPE)          \
        do {                                                    \
            DTYPE * s  = (DTYPE *)SCALING;                        \
            DTYPE * in = (DTYPE *)IN;                             \
            DTYPE * io = (DTYPE *)INOUT;                          \
            for (int i = 0; i < COUNT; i++) io[i] += (*s)*in[i];  \
        } while (0)

#define GALU_AXPBY(S1,IN1,S2,IN2,INOUT,COUNT,DTYPE)                         \
        do {                                                                  \
            DTYPE * s1 = (DTYPE *)S1;                                           \
            DTYPE * s2 = (DTYPE *)S2;                                           \
            DTYPE * in1 = (DTYPE *)IN1;                                         \
            DTYPE * in2 = (DTYPE *)IN2;                                         \
            DTYPE * io = (DTYPE *)INOUT;                                        \
            for (int i = 0; i < COUNT; i++) io[i] = (*s1)*in1[i]+(*s2)*in2[i];  \
        } while (0)

/**************************************************************************
 * GLOBALS
 **************************************************************************/

MPI_Comm GAL_COMM_WORLD;

/**************************************************************************
 * HELPER FUNCTIONS
 **************************************************************************/

//GAL_INLINE int GALI_1D_Lookup(global_array_t * ga, long start_index, long end_index, int * ranks, )
//{
//    assert((ga->ndim)==1);
//
//
//
//    return GAL_SUCCESS;
//}

void * GALU_Malloc(size_t size)
{
    int rc = 0;
    void * ptr = NULL;

#ifdef GAL_MALLOC_ALIGNMENT
    rc = posix_memalign( &ptr, GAL_MALLOC_ALIGNMENT, size);
#else
    ptr = malloc(size);
#endif

    if ( ptr==NULL || rc!=0 )
    {
        int comm_rank = -1;

        MPI_Comm_rank(GAL_COMM_WORLD,&comm_rank);

        if (ptr==NULL)
        {
            fprintf( stderr, "%d: GALU_Malloc resulted in ptr==NULL \n",
                     comm_rank);
        }
#if defined(EINVAL) && defined(ENOMEM)
        if ( rc==EINVAL )
        {
            fprintf( stderr , "%d: GALU_Malloc: posix_memalign returned EINVAL "
                     "(The alignment argument was not a power of two, or was not a multiple of sizeof(void *).). \n",
                     comm_rank);
        }
        else if ( rc==ENOMEM )
        {
            fprintf( stderr , "%d: GALU_Malloc: posix_memalign returned ENOMEM "
                     "(There was insufficient memory to fulfill the allocation request.). \n",
                     comm_rank);
        }
        else
#endif
        if ( rc!=0 )
        {
            fprintf( stderr , "%d: GALU_Malloc: posix_memalign returned a non-zero value (%d). \n",
                     comm_rank, rc);
        }

        fflush(stderr);
        exit(1);
    }

    return ptr;
}

void GALU_Free(void * ptr)
{
    free(ptr);

    return;
}

/**************************************************************************
 * GA-Lite API
 **************************************************************************/

/* collective */
GAL_Result GAL_Initialize(void)
{
    int mpi_status = MPI_SUCCESS;
    int mpi_initialized = 0;

    /* MPI has to be initialized for this implementation to work */
    mpi_status = MPI_Initialized(&mpi_initialized);
    assert( mpi_status==MPI_SUCCESS && mpi_initialized==1 );

    /* to be proper, GAL has to use its own communicator for collectives */
    mpi_status = MPI_Comm_dup(MPI_COMM_WORLD,&GAL_COMM_WORLD);
    assert(mpi_status==MPI_SUCCESS);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(GAL_COMM_WORLD);
    assert(mpi_status==MPI_SUCCESS);
#endif

    return GAL_SUCCESS;
}

/* collective */
GAL_Result GAL_Terminate(void)
{
    int mpi_status = MPI_SUCCESS;

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(GAL_COMM_WORLD);
    assert(mpi_status==MPI_SUCCESS);
#endif

    /* have to use our own communicator for collectives to be proper */
    mpi_status = MPI_Comm_free(&GAL_COMM_WORLD);
    assert(mpi_status==MPI_SUCCESS);

    return GAL_SUCCESS;
}

/**************************************************************************
message  - string to print          [input]
code     - code to print            [input]
local
 ***************************************************************************/
void GAL_Error(char * message, int code)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank;

    mpi_status = MPI_Comm_rank(GAL_COMM_WORLD,&comm_rank);
    assert(mpi_status==MPI_SUCCESS);

    fprintf(stderr, "%d: %s", comm_rank, message );

    mpi_status = MPI_Abort(GAL_COMM_WORLD,code);
    assert(mpi_status==MPI_SUCCESS);

    return;
}

/* collective */
GAL_Result GAL_Sync(void)
{
    int mpi_status = MPI_SUCCESS;

    mpi_status = MPI_Barrier(GAL_COMM_WORLD);
    assert(mpi_status==MPI_SUCCESS);

    return GAL_SUCCESS;
}

/**************************************************************************
      array_name        - a unique character string                   [input]
      type              - data type (MT_F_DBL,MT_F_INT,MT_F_DCPL)     [input]
      ndim              - number of array dimensions                  [input]
      dims[ndim]        - array of dimensions                         [input]
      chunk[ndim]       - array of chunks, each element specifies minimum size that
                          given dimensions should be chunked up into  [input]
Creates an ndim-dimensional array using the regular distribution model 
and returns integer handle representing the array.
The array can be distributed evenly or not. 
The control over the distribution is accomplished by specifying 
chunk (block) size for all or some of array dimensions. 
For example, for a 2-dimensional array, setting chunk[0]=dim[0] 
gives distribution by vertical strips (chunk[0]*dims[0]); 
setting chunk[1]=dim[1] gives distribution by horizontal strips (chunk[1]*dims[1]). 
Actual chunks will be modified so that they are at least the size of the minimum and 
each process has either zero or one chunk. Specifying chunk[i] as <1 will 
cause that dimension to be distributed evenly.

As a convenience, when chunk is specified as NULL, the entire array is distributed evenly.

Return value: a non-zero array handle means the call was succesful.
This is a collective operation.
 ****************************************************************************/

/* collective */
/* seems thread-safe, but need to verify */
GAL_Result GAL_Create(MPI_Comm comm,
                      MPI_Datatype type,
                      int ndim,
                      size_t dimsize[],
                      size_t blocksize[],
                      global_array_t * ga)
{
    int mpi_status = MPI_SUCCESS;
    int comm_size = 0, comm_rank = -1;
    int type_size = 0;
    size_t local_size = 0;
    size_t * new_blocksize = NULL;

    assert(ga->active == 0);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    mpi_status = MPI_Comm_rank( comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Comm_size( comm, &comm_size );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Type_size( type, &type_size );
    assert(mpi_status==MPI_SUCCESS);

    new_blocksize = (size_t*) GALU_Malloc( ndim * sizeof(size_t) );
    assert( new_blocksize!= NULL );

    /* TODO: improve this by determining an optimal blocksize */
    assert( ndim < GAL_DEFAULT_BLOCKSIZE ); /* make sure that blocksize will stay positive definite */
    for( int i=0 ; i<ndim ; i++)
        if ( blocksize[i] < 1 ) blocksize[i] = GAL_DEFAULT_BLOCKSIZE/ndim;

    /* compute local size */
    if (ndim==1)
    {
        size_t remainder = -1;
        size_t num_blocks = -1;

        /* bookkeeping multiple blocks per node is a pain  *
         * TODO: make multiple blocks per node work?      */
        new_blocksize[0] = blocksize[0];
        while (  ( comm_size * new_blocksize[0] ) < dimsize[0] )
            new_blocksize[0] += blocksize[0];

        remainder = dimsize[0] % new_blocksize[0];
        num_blocks = ( dimsize[0] - remainder ) / new_blocksize[0];
        if (remainder>0) num_blocks++;

        if ( (long)comm_rank <= num_blocks )
            local_size = new_blocksize[0]; /* waste memory on last block if remainder>0 to simplify bookkeeping
             * also, this is necessary to use symmetric heap sanely              */
        else /* unnecessary, but pedantic */
            local_size = 0;

#ifdef GAL_DEBUG
        if (comm_rank==0)
        {

        }
#endif
    }
    else
    {
        return GAL_UNIMPLEMENTED;
    }

    ga->comm         = comm;
    ga->type         = type;
    ga->type_size    = type_size;
    ga->ndim         = ndim;
    ga->local_size   = local_size;

    ga->dimsize      = (size_t*) GALU_Malloc( ndim * sizeof(size_t) );
    ga->blocksize    = (size_t*) GALU_Malloc( ndim * sizeof(size_t) );

    memcpy( ga->dimsize,   dimsize,       ndim * sizeof(size_t) );
    memcpy( ga->blocksize, new_blocksize, ndim * sizeof(size_t) );

    GALU_Free(new_blocksize);

    if ( ga->local_size > 0 )
    {
        mpi_status =  MPI_Alloc_mem( (MPI_Aint) (ga->local_size) * (ga->type_size), MPI_INFO_NULL, &(ga->local_buffer) );
        assert(mpi_status==MPI_SUCCESS);
    }

    mpi_status = MPI_Win_create( ga->local_buffer, (MPI_Aint) ga->local_size, ga->type_size, MPI_INFO_NULL, comm, &(ga->window) );
    assert(mpi_status==MPI_SUCCESS);

    ga->active = 1;

    return GAL_SUCCESS;
}

/* collective, not thread-safe */
GAL_Result GAL_Destroy(global_array_t * ga)
{
    int mpi_status = MPI_SUCCESS;

    assert(ga->active == 1);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(ga->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    GALU_Free(ga->dimsize);
    GALU_Free(ga->blocksize);

    mpi_status = MPI_Win_free( &(ga->window) );
    assert(mpi_status==MPI_SUCCESS);

    if ( ga->local_size > 0 )
    {
        mpi_status =  MPI_Free_mem( ga->local_buffer );
        assert(mpi_status==MPI_SUCCESS);
    }

    /* easiest way to annihilate the struct */
    memset( ga, '\0', sizeof(global_array_t) );

    /* just in case memset doesn't set this properly */
    ga->active = 0;

    return GAL_SUCCESS;
}

/* collective, not thread-safe */
GAL_Result GAL_Copy(global_array_t * ga,
                    global_array_t * gb)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank = -1;

    assert(ga->active == 1);
    assert(gb->active == 0);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(ga->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

#if 0
    gb->comm         = ga->comm;
    gb->type         = ga->type;
    gb->type_size    = ga->type_size;
    gb->ndim         = ga->ndim;
    gb->local_size   = ga->local_size;
    memcpy( gb->dimsize,   ga->dimsize,   (ga->ndim)*sizeof(size_t) );
    memcpy( gb->blocksize, ga->blocksize, (ga->ndim)*sizeof(size_t) );
#else
    memcpy( gb, ga, sizeof(global_array_t) );
#endif

    if ( (gb->local_size) > 0 )
    {
        mpi_status =  MPI_Alloc_mem( (MPI_Aint) (gb->local_size) * (gb->type_size), MPI_INFO_NULL, &(gb->local_buffer) );
        assert(mpi_status==MPI_SUCCESS);
    }

    mpi_status = MPI_Comm_rank( gb->comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, comm_rank, /* assert */ 0, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    memcpy( &(gb->local_buffer), &(ga->local_buffer), (gb->local_size) * (gb->type_size) );

    mpi_status = MPI_Win_unlock( comm_rank, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Win_create( gb->local_buffer,
                                 (MPI_Aint) (gb->local_size),
                                 gb->type_size,
                                 MPI_INFO_NULL,
                                 gb->comm,
                                 &(gb->window) );
    assert(mpi_status==MPI_SUCCESS);

    gb->active = 1;

    return GAL_SUCCESS;
}

/* collective */
GAL_Result GAL_Zero(global_array_t * ga)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank = -1;

    assert(ga->active == 1);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(ga->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    mpi_status = MPI_Comm_rank( ga->comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, comm_rank, /* assert */ 0, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    if (ga->type == MPI_DOUBLE) {
        GALU_ZERO( ga->local_buffer, ga->local_size, double );
    } else if (ga->type == MPI_FLOAT) {
        GALU_ZERO( ga->local_buffer, ga->local_size, float );
    } else if (ga->type == MPI_INT) {
        GALU_ZERO( ga->local_buffer, ga->local_size, int );
    } else if (ga->type == MPI_LONG) {
        GALU_ZERO( ga->local_buffer, ga->local_size, long );
    } else {
        if (comm_rank == 0)
            fprintf(stderr, "GAL_Zero: unknown type \n");
        assert(0);
    }

    mpi_status = MPI_Win_unlock( comm_rank, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    return GAL_SUCCESS;
}

/* collective */
GAL_Result GAL_Fill(global_array_t * ga, void * value)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank = -1;

    assert(ga->active == 1);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(ga->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    mpi_status = MPI_Comm_rank( ga->comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, comm_rank, /* assert */ 0, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    if (ga->type == MPI_DOUBLE) {
        GALU_SET( value, ga->local_buffer, ga->local_size, double );
    } else if (ga->type == MPI_FLOAT) {
        GALU_SET( value, ga->local_buffer, ga->local_size, float );
    } else if (ga->type == MPI_INT) {
        GALU_SET( value, ga->local_buffer, ga->local_size, int );
    } else if (ga->type == MPI_LONG) {
        GALU_SET( value, ga->local_buffer, ga->local_size, long );
    } else {
        if (comm_rank == 0)
            fprintf(stderr, "GAL_Fill: unknown type \n");
        assert(0);
    }

    mpi_status = MPI_Win_unlock( comm_rank, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    return GAL_SUCCESS;
}

/* collective */
GAL_Result GAL_Scale(global_array_t * ga, void *value)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank = -1;

    assert(ga->active == 1);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(ga->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    mpi_status = MPI_Comm_rank( ga->comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

    mpi_status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, comm_rank, /* assert */ 0, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    if (ga->type == MPI_DOUBLE) {
        GALU_SCALE( value, ga->local_buffer, ga->local_size, double );
    } else if (ga->type == MPI_FLOAT) {
        GALU_SCALE( value, ga->local_buffer, ga->local_size, float );
    } else if (ga->type == MPI_INT) {
        GALU_SCALE( value, ga->local_buffer, ga->local_size, int );
    } else if (ga->type == MPI_LONG) {
        GALU_SCALE( value, ga->local_buffer, ga->local_size, long );
    } else {
        if (comm_rank == 0)
            fprintf(stderr, "GAL_Scale: unknown type \n");
        assert(0);
    }

    mpi_status = MPI_Win_unlock( comm_rank, ga->window );
    assert(mpi_status==MPI_SUCCESS);

    return GAL_SUCCESS;
}

/**************************************************************************
g_a, g_b, g_c                        - array handles       [input]
double/complex/int      *alpha       - scale factor        [input]
double/complex/int      *beta        - scale factor        [input]
The arrays (which must be the same shape and identically aligned) are added together element-wise
     c = alpha * a  +  beta * b.

This is a collective operation.
 ***************************************************************************/
GAL_Result GAL_Add(void * alpha, global_array_t * ga, void * beta, global_array_t * gb, global_array_t * gc)
{
    int mpi_status = MPI_SUCCESS;
    int comm_rank = -1;
    int comm_result = -1;
    int unequal = 0;

    assert(ga->active == 1);
    assert(gb->active == 1);
    assert(gc->active == 1);

    /* here we assume this is at least collective over gc->comm,
     * in the event non-congruent use is supported               */
    mpi_status = MPI_Comm_rank( gc->comm, &comm_rank );
    assert(mpi_status==MPI_SUCCESS);

#ifdef GAL_COLLECTIVES_HAVE_BARRIER_SEMANTICS
    mpi_status = MPI_Barrier(gc->comm);
    assert(mpi_status==MPI_SUCCESS);
#endif

    MPI_Comm_compare( ga->comm, gb->comm, &comm_result );
    unequal += ( comm_result==MPI_IDENT || comm_result==MPI_CONGRUENT );
    MPI_Comm_compare( gb->comm, gc->comm, &comm_result );
    unequal += ( comm_result==MPI_IDENT || comm_result==MPI_CONGRUENT );

    unequal += ( ga->type != gb->type );
    unequal += ( gb->type != gc->type );
    unequal += ( ga->ndim != gb->ndim );
    unequal += ( gb->ndim != gc->ndim );
    unequal += ( ga->type_size != gb->type_size );
    unequal += ( gb->type_size != gc->type_size );
    unequal += ( ga->local_size != gb->local_size );
    unequal += ( gb->local_size != gc->local_size );
    for (int i = 0 ; i < (ga->ndim) ; i++ )
    {
        unequal += ( ga->dimsize[i]   != gb->dimsize[i]   );
        unequal += ( gb->dimsize[i]   != gc->dimsize[i]   );
        unequal += ( ga->blocksize[i] != gb->blocksize[i] );
        unequal += ( gb->blocksize[i] != gc->blocksize[i] );
    }

    if(unequal==0)
    {
        mpi_status = MPI_Win_lock( MPI_LOCK_EXCLUSIVE, comm_rank, /* assert */ 0, ga->window );
        assert(mpi_status==MPI_SUCCESS);

        if (ga->type == MPI_DOUBLE) {
            GALU_AXPBY( alpha, ga->local_buffer, beta, gb->local_buffer, gc->local_buffer, ga->local_size, double );
        } else if (ga->type == MPI_FLOAT) {
            GALU_AXPBY( alpha, ga->local_buffer, beta, gb->local_buffer, gc->local_buffer, ga->local_size, float );
        } else if (ga->type == MPI_INT) {
            GALU_AXPBY( alpha, ga->local_buffer, beta, gb->local_buffer, gc->local_buffer, ga->local_size, int );
        } else if (ga->type == MPI_LONG) {
            GALU_AXPBY( alpha, ga->local_buffer, beta, gb->local_buffer, gc->local_buffer, ga->local_size, long );
        } else {
            if (comm_rank == 0)
                fprintf(stderr, "GAL_Add: unknown type \n");
            assert(0);
        }

        mpi_status = MPI_Win_unlock( comm_rank, ga->window );
        assert(mpi_status==MPI_SUCCESS);

    } else {
        /* TODO generalize implementation to fall back to crappy communication-based algorithm when unequal */
        if (comm_rank == 0)
            fprintf(stderr, "GAL_Add: not implemented when arrays lack identical layout \n");

        return GAL_UNIMPLEMENTED;
    }

    return GAL_SUCCESS;
}

/**************************************************************************
g_a        - global array handle                                                  [input]
ndim       - number of dimensions of the global array
lo[ndim]   - array of starting indices for global array section                   [input]
hi[ndim]   - array of ending indices for global array section                     [input]
buf        - pointer to the local buffer array where the data goes                [output]
ld[ndim-1] - array specifying leading dimensions/strides/extents for buffer array [input]

Copies data from global array section to the local array buffer.
The local array is assumed to be have the same number of dimensions as the global array.
Any detected inconsitencies/errors in the input arguments are fatal.

Example:
For ga_get operation transfering data from the [10:14,0:4] section of
2-dimensional 15x10 global array into local buffer 5x10 array we have:
lo={10,0}, hi={14,4}, ld={10}
 ***************************************************************************/
GAL_Result GAL_Get(global_array_t * ga, long lo[], long hi[], long ld[], void * buffer )
{
#ifdef GAL_GET_DONE
    int mpi_status = MPI_SUCCESS;

    /* TODO implement lookup for which ranks own data */

    /* TODO implement lookup for offset of data within those ranks */

    if ((ga->ndim)==1)
    {
        long origin_count = hi[0] - lo[0];
        int rank = (int)( lo[0] / ga->blocksize[0] );

        while (origin_count > 0)
        {
            mpi_status = MPI_Get( &buffer,
                                  origin_count,
                                  ga->type,
                                  target_rank,
                                  (MPI_Aint) ga->type_size,
                                  target_count,
                                  ga->type,
                                  ga->window );
            assert(mpi_status==MPI_SUCCESS);
        }
    }
    else
    {
        return GAL_UNIMPLEMENTED;
    }
#endif

    return GAL_UNIMPLEMENTED;
}

/**************************************************************************
g_a        - global array handle                                                  [output]
ndim       - number of dimensions of the global array
lo[ndim]   - array of starting indices for global array section                   [input]
hi[ndim]   - array of ending indices for global array section                     [input]
buf        - pointer to the local buffer array where the data is                  [input]
ld[ndim-1] - array specifying leading dimensions/strides/extents for buffer array [input]

Copies data from local array buffer to the global array section.
The local array is assumed to be have the same number of dimensions as the global array.
Any detected inconsistencies/errors in input arguments are fatal.
 ***************************************************************************/
GAL_Result GAL_Put(global_array_t * ga, int lo[], int hi[], void* buf, int ld[])
{
    return GAL_UNIMPLEMENTED;
}

/**************************************************************************
g_a        - global array handle                                                  [input]
ndim       - number of dimensions of the global array
lo[ndim]   - array of starting indices for array section                          [input]
hi[ndim]   - array of ending indices for array section                            [input]
buf        - pointer to the local buffer array                                    [input]
ld[ndim-1] - array specifying leading dimensions/strides/extents for buffer array [input]
double/DoubleComplex/long *alpha     scale factor                                 [input]

Combines data from local array buffer with data in the global array section.
The local array is assumed to be have the same number of dimensions as the global array.
global array section (lo[],hi[]) += *alpha * buffer
 ***************************************************************************/
GAL_Result GAL_Acc(global_array_t * ga, int lo[], int hi[], void* buf, int ld[], void * alpha)
{
    return GAL_UNIMPLEMENTED;
}

/**************************************************************************
g_a              array handle         [input]
subscript[ndim]  element subscript    [output]
Return in owner the GA compute process id that 'owns' the data.
If any element of subscript[] is out of bounds "-1" is returned.
This operation is local.
 ***************************************************************************/
GAL_Result GAL_Locate(global_array_t * ga, int subscript[], int * rank)
{
    return GAL_UNIMPLEMENTED;
}

/**************************************************************************
      g_a           - global array handle                               [input]
      ndim          - number of dimensions of the global array
      lo[ndim]      - array of starting indices for array section       [input]
      hi[ndim]      - array of ending indices for array section         [input]
      map[][2*ndim] - array with mapping information                    [output]
      procs[nproc]  - list of processes that own a part of array section[output]

Return the list of the GA processes id that 'own' the data.
Parts of the specified patch might be actually 'owned' by several processes.
If lo/hi are out of bounds "0" is returned, otherwise
return value is equal to the number of processes that hold the data .

        map[i][0:ndim-1]         - lo[i]
        map[i][ndim:2*ndim-1]    - hi[i]
        procs[i]                 - processor id that owns data in patch lo[i]:hi[i]

This operation is local.
 ***************************************************************************/
GAL_Result GAL_Locate_region(global_array_t * ga, int lo[], int hi[], int map[], int procs[])
{
    return GAL_UNIMPLEMENTED;
}

/**************************************************************************
g_a  - array handle                  [input]
type - data type                     [output]
ndim - number of dimensions          [output]
dims - array of dimensions           [output]
 ***************************************************************************/
GAL_Result GAL_Inquire(global_array_t * ga, MPI_Datatype * type, int * ndim, size_t dims[])
{
    (*type) = ga->type;
    (*ndim) = ga->ndim;
    memcpy( dims,   ga->dimsize, (*ndim)*sizeof(size_t) );

    return GAL_SUCCESS;
}

/**************************************************************************
g_a        - array handle                                [input]
iproc      - process number                              [input]
ndim       - number of dimensions of the global array
lo[ndim]   - array of starting indices for array section [input]
hi[ndim]   - array of ending indices for array section   [input]
If no array elements are owned by process iproc,
the range is returned as lo[ ]=0 and hi[ ]= -1 for all dimensions.

This operation is local.
 ***************************************************************************/
GAL_Result GAL_Distribution(global_array_t * ga, int proc, int lo[], int hi[])
{
    return GAL_UNIMPLEMENTED;
}
