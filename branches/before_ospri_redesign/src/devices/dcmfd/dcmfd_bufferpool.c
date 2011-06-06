/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

OSPD_Buffer_pool_t OSPD_Buffer_pool;

OSPD_Buffer_t* OSPDI_Get_buffer(int size, int wait_and_advance)
{
    int status = OSP_SUCCESS;
    OSPD_Buffer_t *ospd_buffer = NULL;
    void* new_buffer;
    int index;

    OSPU_FUNC_ENTER();

    /* Assumes sizes array is sorted in increasing order, if requested buffer is 
       larger than the largest buffer possible, allocate a fresh buffer and return 
       it */
    if(size > OSPD_Buffer_pool.sizes[OSPC_BUFFER_SIZES-1]) 
    {
        status = OSPDI_Malloc((void **) &new_buffer, sizeof(OSPD_Buffer_t) + size);
        OSPU_ERR_POP(status != OSP_SUCCESS,
                    "OSPDI_Malloc return with an error \n");
        ospd_buffer = (OSPD_Buffer_t *) new_buffer;
        ospd_buffer->buffer_ptr = (void *) ((size_t) new_buffer + sizeof(OSPD_Buffer_t));
        ospd_buffer->pool_index = -1;
        return ospd_buffer;
    }

    do { 
        for(index=0; index<OSPC_BUFFER_SIZES; index++) 
        {
           if((size <= OSPD_Buffer_pool.sizes[index]) && OSPD_Buffer_pool.pool_heads[index] != NULL)
           {
              ospd_buffer = OSPD_Buffer_pool.pool_heads[index];
              OSPD_Buffer_pool.pool_heads[index] = OSPD_Buffer_pool.pool_heads[index]->next; 
              ospd_buffer->pool_index = index;
              return ospd_buffer;
            } 
         }

         OSPU_DEBUG_PRINT("Buffer pool exhausted. Looking for buffer of size: %d.\
                Wait and advance is: %d \n", size, wait_and_advance);

         /* If you are allowed to hit advance, do so and wait until a buffer is free. Or else, 
            return by allocating a new buffer */ 
         if(wait_and_advance) 
         {
             OSPDI_Advance();
         }
         else
         {
              void* new_buffer;
              status = OSPDI_Malloc((void **) &new_buffer, sizeof(OSPD_Buffer_t) + size);
              OSPU_ERR_POP(status != OSP_SUCCESS,
                    "OSPDI_Malloc return with an error \n");
              ospd_buffer = (OSPD_Buffer_t *) new_buffer;
              ospd_buffer->buffer_ptr = (void *) ((size_t) new_buffer + sizeof(OSPD_Buffer_t));
              ospd_buffer->pool_index = -1;
              return ospd_buffer;
         }
     } while(ospd_buffer == NULL);	

  fn_exit: 
    OSPU_FUNC_EXIT();
    return ospd_buffer;

  fn_fail: 
    goto fn_exit;
}

void OSPDI_Release_buffer(OSPD_Buffer_t *ospd_buffer)
{
    OSPU_FUNC_ENTER();

    if(ospd_buffer->pool_index == -1) 
    {
       OSPDI_Free(ospd_buffer);
    }
    else
    {
       ospd_buffer->next = OSPD_Buffer_pool.pool_heads[ospd_buffer->pool_index];
       OSPD_Buffer_pool.pool_heads[ospd_buffer->pool_index] = ospd_buffer;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return;

  fn_fail:
    goto fn_exit;
}

int OSPDI_Buffer_pool_initialize()
{
    int status = OSP_SUCCESS;
    int i, j;
    OSPD_Buffer_t *ospd_buffer = NULL;

    OSPU_FUNC_ENTER();

    /*TODO: We should make these assignments dynamic*/
    OSPD_Buffer_pool.sizes[0] = ospd_settings.put_packetsize;
    OSPD_Buffer_pool.sizes[1] = ospd_settings.get_packetsize;
    OSPD_Buffer_pool.sizes[2] = ospd_settings.putacc_packetsize;

    OSPD_Buffer_pool.limits[0] = ospd_settings.put_bufferpool_size;
    OSPD_Buffer_pool.limits[1] = ospd_settings.get_bufferpool_size;
    OSPD_Buffer_pool.limits[2] = ospd_settings.putacc_bufferpool_size;    

    for(i=0; i<OSPC_BUFFER_SIZES; i++)
    {
        /* Initializing Put and Get buffer pool */
        status = OSPDI_Malloc((void **) &(OSPD_Buffer_pool.pool_region_ptrs[i]), 
                                     sizeof(OSPD_Buffer_t) * OSPD_Buffer_pool.limits[i]);
        OSPU_ERR_POP(status != OSP_SUCCESS,
                    "OSPDI_Malloc failed while allocating request pool\
                          in OSPDI_Buffer_pool_initialize\n");
  
        status = OSPDI_Malloc((void **) &(OSPD_Buffer_pool.mem_region_ptrs[i]),
                                     OSPD_Buffer_pool.sizes[i]  * OSPD_Buffer_pool.limits[i]);
        OSPU_ERR_POP(status != OSP_SUCCESS,
                    "OSPDI_Malloc failed while allocating request pool\
                          in OSPDI_Buffer_pool_initialize\n");
  
        ospd_buffer = OSPD_Buffer_pool.pool_region_ptrs[i];
        OSPD_Buffer_pool.pool_heads[i] = ospd_buffer;
        for (j=0; j<OSPD_Buffer_pool.limits[i]-1; j++)
        {
            ospd_buffer[j].next = &ospd_buffer[j+1];
            ospd_buffer[j].buffer_ptr = (void *) ((size_t) OSPD_Buffer_pool.mem_region_ptrs[i] + 
                                          j * OSPD_Buffer_pool.sizes[i]);
            ospd_buffer[j].pool_index == i;
        }
        ospd_buffer[j].next = NULL;
        ospd_buffer[j].buffer_ptr = (void *) ((size_t) OSPD_Buffer_pool.mem_region_ptrs[i] +
                                      (OSPD_Buffer_pool.limits[i]-1) * OSPD_Buffer_pool.sizes[i]);
        ospd_buffer[j].pool_index == i;
     }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

void OSPDI_Buffer_pool_finalize()
{
    int i;

    OSPU_FUNC_ENTER();

    for(i=0; i<OSPC_BUFFER_SIZES; i++)
    {
       OSPDI_Free((void *) (OSPD_Buffer_pool.pool_region_ptrs[i]));
       OSPDI_Free((void *) (OSPD_Buffer_pool.mem_region_ptrs[i]));
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return;

  fn_fail:
    goto fn_exit;
}
