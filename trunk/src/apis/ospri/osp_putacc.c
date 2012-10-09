/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_PutAcc(int target,
              void* source_ptr,
              void* target_ptr,
              int bytes,
              OSP_datatype_t osp_type,
              void* scaling)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTACC, target, bytes);
    }
#   endif

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_Acc_local(source_ptr,
                                target_ptr,
                                bytes,
                                osp_type,
                                scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Acc_local returned an error\n");
    }
    else
    {
        status = OSPD_PutAcc(target,
                            source_ptr,
                            target_ptr,
                            bytes,
                            osp_type,
                            scaling);
        OSPU_ERR_POP((status!=OSP_SUCCESS), "OSPD_PutAcc returned error\n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSP_NbPutAcc(int target,
                void* source_ptr,
                void* target_ptr,
                int bytes,
                OSP_datatype_t osp_type,
                void* scaling,
                OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUTACC, target, bytes);
    }
#   endif

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_Acc_local(source_ptr,
                                target_ptr,
                                bytes,
                                osp_type,
                                scaling);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Acc_local returned an error\n");
    }
    else
    {
        status = OSPD_NbPutAcc(target,
                              source_ptr,
                              target_ptr,
                              bytes,
                              osp_type,
                              scaling,
                              osp_handle);
        OSPU_ERR_POP((status!=OSP_SUCCESS), "OSPD_NbPutAcc returned error\n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}
