/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

/* This is here because the build system does not yet have the necessary
 * logic to set these options for each device. */
#define OSPD_IMPLEMENTS_PUTMODV

#if defined OSPD_IMPLEMENTS_PUTMODV

int OSP_PutModV(int target,
               OSP_iov_t *iov_ar,
               int ar_len,
               OSP_reduce_op_t osp_op,
               OSP_datatype_t osp_type)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        int i, total_bytes = 0;
        for (i = 0; i < ar_len; i++)
            total_bytes += iov_ar[i].ptr_array_len * iov_ar[i].bytes;
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUTMODV, target, total_bytes);
    }
#   endif

    /* Bypass is ALWAYS better for accumulate; we do not test against threshold. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_ModV_memcpy(iov_ar, ar_len, osp_op, osp_type);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_ModV_memcpy returned an error\n");
    }
    else
    {
        status = OSPD_PutModV(target, iov_ar, ar_len, osp_op, osp_type);
        OSPU_ERR_POP(status, "OSPD_PutModV returned error\n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

#else

int OSP_PutModV(int target,
        OSP_iov_t *iov_ar,
        int ar_len,
        OSP_reduce_op_t osp_op,
        OSP_datatype_t osp_type);
{
    int i, j, status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSP_PutModV requires device-level implementation.\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#endif /* OSPD_IMPLEMENTS_PUTMODV */
