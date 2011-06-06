/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

int OSPD_Wait_handle_all()
{
    int status = OSP_SUCCESS;
    int index;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    for (index = 0; index < ospd_settings.handlepool_size; index++)
    {
        if (OSPD_Active_handle_list[index] != NULL)
            OSPDI_Conditional_advance((OSPD_Active_handle_list[index])->active > 0);
    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Wait_handle(OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    OSPDI_Conditional_advance(ospd_handle->active > 0);

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Wait_handle_list(int count, OSP_handle_t *osp_handle)
{
    int status = OSP_SUCCESS;
    int index;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    for (index = 0; index < count; index++)
    {
        /* TODO: Do we really need to do a copy here?
         *        Isn't it enough to pass the argument as const?
         */
        ospd_handle = (OSPD_Handle_t *) osp_handle[index];
        OSPDI_Conditional_advance(ospd_handle->active > 0);
    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Test_handle(OSP_handle_t osp_handle, OSP_bool_t* completed)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPDI_Advance();
    /* TODO: Do we really need to do a copy here?
     *        Isn't it enough to pass the argument as const?
     */
    ospd_handle = (OSPD_Handle_t *) osp_handle;
    *completed = (ospd_handle->active > 0) ? OSP_FALSE : OSP_TRUE;

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_Test_handle_list(int count,
                         OSP_handle_t *osp_handle,
                         OSP_bool_t* *completed)
{
    int i;
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    /* TODO: verify that this is the correct implementation */
    OSPDI_Advance();
    for (i = 0; i < count; i++)
    {
        /* TODO: Do we really need to do a copy here?
         *        Isn't it enough to pass the argument as const?
         */
        ospd_handle = (OSPD_Handle_t *) osp_handle[i];
        *completed[i] = (ospd_handle->active > 0) ? OSP_FALSE : OSP_TRUE;
    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}
