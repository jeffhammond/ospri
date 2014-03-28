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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <dcmf.h>
#include <dcmf_collectives.h>

#define MAX_MSG_SIZE 1024*1024

DCMF_Geometry_t geometry;

DCMF_Geometry_t *getGeometry (int comm)
{
    return &geometry;
}

void done(void *clientdata, DCMF_Error_t *error)
{
    --(*((uint32_t *) clientdata));
}

int main()
{
    DCMF_Result status;

    DCMF_Messager_initialize();
    DCMF_Collective_initialize();

    int rank = DCMF_Messager_rank();
    int nranks = DCMF_Messager_size();

    /* barrier and geometry setup */

    /* used temporarily by both barrier protocols */
    DCMF_Barrier_Configuration_t barrier_conf;

    DCMF_CollectiveProtocol_t barrier_protocol;
    barrier_conf.protocol = DCMF_GI_BARRIER_PROTOCOL;
    barrier_conf.cb_geometry = getGeometry; 
    status = DCMF_Barrier_register( &barrier_protocol, &barrier_conf);
    assert( status == DCMF_SUCCESS );

    DCMF_CollectiveProtocol_t lbarrier_protocol;
    barrier_conf.protocol = DCMF_LOCKBOX_BARRIER_PROTOCOL;
    barrier_conf.cb_geometry = getGeometry;
    status = DCMF_Barrier_register( &lbarrier_protocol, &barrier_conf);
    assert( status == DCMF_SUCCESS );

    unsigned * ranks = (unsigned *) malloc(nranks * sizeof(int)); assert(ranks!=NULL);
    for(int i=0; i<nranks; i++)
        ranks[i] = i;

    DCMF_CollectiveProtocol_t  *barrier_ptr, *lbarrier_ptr;
    barrier_ptr   = &barrier_protocol;
    lbarrier_ptr  = &lbarrier_protocol;

    DCMF_CollectiveRequest_t crequest;
    status = DCMF_Geometry_initialize(&geometry, 0, ranks, nranks, &barrier_ptr, 1, &lbarrier_ptr, 1, &crequest, 0, 1);
    assert( status == DCMF_SUCCESS );

    /* allreduce setup */

#if 0
DCMF_TREE_ALLREDUCE_PROTOCOL                        Tree allreduce.  
DCMF_TREE_PIPELINED_ALLREDUCE_PROTOCOL              Tree allreduce.  
DCMF_TREE_DPUT_PIPELINED_ALLREDUCE_PROTOCOL         Tree allreduce.  
DCMF_TORUS_BINOMIAL_ALLREDUCE_PROTOCOL              Torus binomial allreduce.  
DCMF_TORUS_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL        Torus binomial short allreduce.  
DCMF_TORUS_ASYNC_BINOMIAL_ALLREDUCE_PROTOCOL        Torus binomial allreduce.  
DCMF_TORUS_ASYNC_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL  Torus binomial short async allreduce.  
DCMF_TORUS_RECTANGLE_ALLREDUCE_PROTOCOL             Torus rectangle/binomial allreduce.  
DCMF_TORUS_RECTANGLE_RING_ALLREDUCE_PROTOCOL        Torus rectangle/ring allreduce.  
DCMF_TORUS_ASYNC_RECTANGLE_ALLREDUCE_PROTOCOL       Torus rectangle/binomial async allreduce.  
DCMF_TORUS_ASYNC_RECTANGLE_RING_ALLREDUCE_PROTOCOL  Torus rectangle/ring async allreduce.  
DCMF_TORUS_ASYNC_SHORT_RECTANGLE_ALLREDUCE_PROTOCOL Torus rectangle short async allreduce.
#endif

    /* used temporarily by both allreduce protocols */
    DCMF_Allreduce_Configuration_t allreduce_conf;

    DCMF_CollectiveProtocol_t allreduce_protocol1;
    allreduce_conf.protocol = DCMF_TORUS_ASYNC_SHORT_RECTANGLE_ALLREDUCE_PROTOCOL;
    allreduce_conf.cb_geometry = getGeometry;
    allreduce_conf.reuse_storage = 1;
    status = DCMF_Allreduce_register( &allreduce_protocol1, &allreduce_conf);
    assert( status == DCMF_SUCCESS );

    if (!DCMF_Geometry_analyze(&geometry, &allreduce_protocol1))
    {
        printf("Not a supported geometry!! \n");
        fflush(stdout);
        return -1;
    }

    DCMF_CollectiveProtocol_t allreduce_protocol2;
    allreduce_conf.protocol = DCMF_TORUS_BINOMIAL_ALLREDUCE_PROTOCOL;
    allreduce_conf.cb_geometry = getGeometry;
    allreduce_conf.reuse_storage = 1;
    status = DCMF_Allreduce_register( &allreduce_protocol2, &allreduce_conf);
    assert( status == DCMF_SUCCESS );

    if (!DCMF_Geometry_analyze(&geometry, &allreduce_protocol2))
    {
        printf("Not a supported geometry!! \n");
        fflush(stdout);
        return -1;
    }

    /* allreduce_active would go in the nonblocking request, along with the DCMF request noted below */
    volatile unsigned allreduce_active;

    DCMF_Callback_t done_callback;
    done_callback.function = done;
    done_callback.clientdata = (void *) &allreduce_active;

    if (rank == 0)
    {
        printf("DCMF_Allreduce Test\n");
        fflush(stdout);
    }

    size_t bufsize = MAX_MSG_SIZE;
    long long * src_buffer = (long long *) malloc(bufsize); assert(src_buffer!=NULL);
    long long * trg_buffer = (long long *) malloc(bufsize); assert(trg_buffer!=NULL);

    for (size_t msgsize = sizeof(long long); msgsize < MAX_MSG_SIZE; msgsize *= 2)
    {
        /*initializing buffer*/
        for (int i = 0; i < bufsize/sizeof(long long); i++)
        {
            src_buffer[i] = rank;
            trg_buffer[i] = 0;
        }

        allreduce_active = 1;

        /* this state has to be on the heap in the nonblocking version because it must not
           be deallocated until the collective has finished */
        DCMF_CollectiveRequest_t crequest1;

        /*sum reduce operation*/
        status = DCMF_Allreduce(&allreduce_protocol1,
                                &crequest1,
                                done_callback,
                                DCMF_RELAXED_CONSISTENCY,
                                &geometry,
                                (char *) src_buffer,
                                (char *) trg_buffer,
                                msgsize/sizeof(long long),
                                DCMF_SIGNED_LONG_LONG,
                                DCMF_SUM);
        assert( status == DCMF_SUCCESS );

        /* this is what the nonblocking wait would do */
        while(allreduce_active > 0) DCMF_Messager_advance();

        long long expected = (nranks-1)*(nranks)/2;
        for (int i = 0; i < msgsize/sizeof(long long); i++)
        {
            if (trg_buffer[i] - expected != 0)
            {
                printf("[%d] Validation has failed Expected: %lld, Actual: %lld, i: %d \n",
                        rank, expected, trg_buffer[i], i);
                fflush(stdout);
                exit(-1);
            }
        }

        printf("[%d] %d message sum allreduce successful \n", rank, msgsize);
        fflush(stdout);

        for (int i = 0; i < bufsize/sizeof(long long); i++)
        {
            src_buffer[i] = rank;
            trg_buffer[i] = 0;
        }

        allreduce_active = 1;

        /* this state has to be on the heap in the nonblocking version because it must not
           be deallocated until the collective has finished */
        DCMF_CollectiveRequest_t crequest2;

        /*sum reduce operation*/
        status = DCMF_Allreduce(&allreduce_protocol2,
                                &crequest2,
                                done_callback,
                                DCMF_RELAXED_CONSISTENCY,
                                &geometry,
                                (char *) src_buffer,
                                (char *) trg_buffer,
                                msgsize/sizeof(long long),
                                DCMF_SIGNED_LONG_LONG,
                                DCMF_SUM);
        assert( status == DCMF_SUCCESS );

        /* this is what the nonblocking wait would do */
        while ( allreduce_active > 0 ) DCMF_Messager_advance();

        for (int i = 0; i < msgsize/sizeof(long long); i++)
        {
            if ( trg_buffer[i] - expected != 0 )
            {
                printf("[%d] Validation has failed Expected: %lld, Actual: %lld, i: %d \n",
                        rank, expected, trg_buffer[i], i);
                fflush(stdout);
                exit(-1);
            }
        }

        printf("[%d] %d message product allreduce successful\n", rank, msgsize);
        fflush(stdout);

    }

    free(src_buffer);
    free(trg_buffer);

    DCMF_Messager_finalize();

    return 0;
}
