/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "pthreaddimpl.h"

int OSPDI_GlobalBarrier()
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(status != 0, " returned with an error");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;

}

int OSPD_Barrier_group(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_GlobalBarrier();
        OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSPDI_GlobalBarrier returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_Barrier_group not implemented for non-world groups!");
        goto fn_fail;
    }


  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;

}


int OSPD_Sync_group(OSP_group_t* group)
{

    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_Flush_all();
        OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSPDI_Flush_all returned with an error");
        status = OSPDI_GlobalBarrier();
        OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSPDI_GlobalBarrier returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_Sync_group not implemented for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;

}
