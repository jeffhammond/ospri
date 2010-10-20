/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

OSPD_Handle_pool_t OSPD_Handle_pool;
OSPD_Handle_t **OSPD_Active_handle_list;

int OSPD_Allocate_handle(OSP_handle_t *osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_Allocate_handle.\n");
    *osp_handle = (OSP_handle_t) ospd_handle;

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Release_handle(OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPU_ASSERT(osp_handle != NULL, status)

    ospd_handle = (OSPD_Handle_t *) osp_handle;
    OSPDI_Release_handle(ospd_handle);

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

OSPD_Handle_t* OSPDI_Get_handle()
{
    OSPD_Handle_t *ospd_handle = NULL;
    int index;

    OSPU_FUNC_ENTER();

    if (OSPD_Handle_pool.head == NULL)
    {
        return NULL;
    }

    ospd_handle = OSPD_Handle_pool.head;
    OSPD_Handle_pool.head = OSPD_Handle_pool.head->next;

    ospd_handle->active = 0;

    /* The size of active handle list is equal to handle pool size,
     * So we should find a free index if we had got a handle above.*/
    index = 0;
    while (OSPD_Active_handle_list[index] != NULL)
        index++;
    OSPD_Active_handle_list[index] = ospd_handle;
    ospd_handle->active_list_index = index;

  fn_exit: 
    OSPU_FUNC_EXIT();
    return ospd_handle;

  fn_fail: 
    goto fn_exit;
}

void OSPDI_Release_handle(OSPD_Handle_t *ospd_handle)
{
    OSPU_FUNC_ENTER();

    OSPDI_Conditional_advance(ospd_handle->active > 0);

    OSPD_Active_handle_list[ospd_handle->active_list_index] = NULL;
    ospd_handle->active_list_index = -1;

    ospd_handle->next = OSPD_Handle_pool.head;
    OSPD_Handle_pool.head = ospd_handle;

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int OSPDI_Handle_pool_initialize()
{

    int status = OSP_SUCCESS;
    int index;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    status = OSPDI_Malloc((void **) &ospd_handle, sizeof(OSPD_Handle_t)
            * ospd_settings.handlepool_size);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc failed while allocating handle pool\
                      in OSPDI_Handle_pool_initialize\n");

    OSPD_Handle_pool.region_ptr = (void *) ospd_handle;
    OSPD_Handle_pool.head = ospd_handle;
    for (index = 0; index < ospd_settings.handlepool_size; index++)
    {
        ospd_handle[index].next = &ospd_handle[index + 1];
    }
    ospd_handle[index].next = NULL;

    status = OSPDI_Malloc((void **) &OSPD_Active_handle_list,
                                 sizeof(OSPD_Handle_t *)
                                         * ospd_settings.handlepool_size);
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc failed in OSPDI_Handle_pool_initialize\n");

    for (index = 0; index < ospd_settings.handlepool_size; index++)
    {
        OSPD_Active_handle_list[index] = NULL;
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void OSPDI_Handle_pool_finalize()
{
    int index;

    OSPU_FUNC_ENTER();

    for (index = 0; index < ospd_settings.handlepool_size; index++)
    {
        if (OSPD_Active_handle_list[index] != NULL)
            OSPDI_Conditional_advance((OSPD_Active_handle_list[index])->active > 0);
    }

    OSPDI_Free(OSPD_Active_handle_list);

    OSPDI_Free(OSPD_Handle_pool.region_ptr);

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}
