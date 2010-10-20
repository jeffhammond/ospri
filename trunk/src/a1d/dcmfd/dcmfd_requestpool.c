/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

OSPD_Request_pool_t OSPD_Request_pool;

OSPD_Request_t* OSPDI_Get_request(int wait_and_advance)
{
    int status = OSP_SUCCESS;
    OSPD_Request_t *ospd_request = NULL;

    OSPU_FUNC_ENTER();

    if(!OSPD_Request_pool.head)
         OSPU_DEBUG_PRINT("Request pool exhausted. Wait and advance is: %d \n", 
                   wait_and_advance);

    if (!wait_and_advance && !OSPD_Request_pool.head)
    {
        status = OSPDI_Malloc((void **) &ospd_request, sizeof(OSPD_Request_t));
        OSPU_ERR_ABORT(status != 0,
                   "OSPDI_Malloc failed while allocating request pool\
                        in OSPDI_Get_request\n");
        ospd_request->in_pool = 0;
    }
    else
    {
        OSPDI_Conditional_advance(!OSPD_Request_pool.head);

        ospd_request = OSPD_Request_pool.head;
        OSPD_Request_pool.head = OSPD_Request_pool.head->next;
        ospd_request->in_pool = 1;
    }

    ospd_request->next = NULL;
    ospd_request->buffer_ptr = NULL;
    ospd_request->ospd_buffer_ptr = NULL;
    ospd_request->handle_ptr = NULL;

    fn_exit: OSPU_FUNC_EXIT();
    return ospd_request;

    fn_fail: goto fn_exit;
}

void OSPDI_Release_request(OSPD_Request_t *ospd_request)
{
    OSPU_FUNC_ENTER();
    OSPD_Handle_t *ospd_handle;

    if (ospd_request->ospd_buffer_ptr != NULL)
    {
        OSPDI_Release_buffer(ospd_request->ospd_buffer_ptr);
    }

    if ((ospd_request->buffer_ptr) != NULL)
    {
        OSPDI_Free((void *) (ospd_request->buffer_ptr));
    }

    if (ospd_request->handle_ptr != NULL)
    {
        ospd_handle = ospd_request->handle_ptr;
        --(ospd_handle->active);
    }

    if (ospd_request->in_pool == 0)
    {
        OSPDI_Free(ospd_request);
    }
    else
    {
        ospd_request->next = OSPD_Request_pool.head;
        OSPD_Request_pool.head = ospd_request;
    }

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int OSPDI_Request_pool_initialize()
{

    int status = OSP_SUCCESS;
    int index;
    OSPD_Request_t *ospd_request;

    OSPU_FUNC_ENTER();

    status = OSPDI_Malloc((void **) &(OSPD_Request_pool.region_ptr),
                                 sizeof(OSPD_Request_t)
                                         * ospd_settings.requestpool_size);
    OSPU_ERR_POP(status != OSP_SUCCESS,
                "OSPDI_Malloc failed while allocating request pool\
                      in OSPDI_Request_pool_initialize\n");

    ospd_request = OSPD_Request_pool.region_ptr;
    OSPD_Request_pool.head = ospd_request;
    for (index = 0; index < ospd_settings.requestpool_size - 1; index++)
    {
        ospd_request[index].next = &ospd_request[index + 1];
    }
    ospd_request[index].next = NULL;

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void OSPDI_Request_pool_finalize()
{
    int i;

    OSPU_FUNC_ENTER();

    OSPDI_Free((void *) (OSPD_Request_pool.region_ptr));

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}
