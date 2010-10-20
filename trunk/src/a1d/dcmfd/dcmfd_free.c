/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

int OSPD_Release_segments(OSP_group_t* group, void *ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    /*This functions does nothing becuase BG does not involve
      any registration. It has to do a barrier syncrhonization
      to ensure everyone is agreeing on the release*/

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_GlobalBarrier();
        OSPU_ERR_ABORT(status != OSP_SUCCESS, "DCMF_GlobalBarrier returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_Release_segments not implemented for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSPD_Free_segment(void *ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    OSPDI_Free(ptr);

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
