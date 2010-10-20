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
#define OSPD_IMPLEMENTS_GETV

#if defined OSPD_IMPLEMENTS_GETV

int OSP_GetV(int target, OSP_iov_t *iov_ar, int ar_len)
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
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_GETV, target, total_bytes);
    }
#   endif

    /* It isn't worth trying to optimize for the threshold here because
     * these operations aren't used much in GA. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_GetV_memcpy(iov_ar, ar_len);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_GetV_memcpy returned error\n");
    }
    else
    {
        status = OSPD_GetV(target, iov_ar, ar_len);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_GetV returned error\n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSP_NbGetV(int target, OSP_iov_t *iov_ar, int ar_len, OSP_handle_t osp_handle)
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
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_GETV, target, total_bytes);
    }
#   endif

    /* It isn't worth trying to optimize for the threshold here because
     * these operations aren't used much in GA. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_GetV_memcpy(iov_ar, ar_len);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_GetV_memcpy returned error\n");
    }
    else
    {
        status = OSPD_NbGetV(target, iov_ar, ar_len, osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbGetV returned error\n");
    }

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

#else

int OSP_GetV(int target,
        OSP_iov_t *iov_ar,
        int ar_len)
{
    int i, j, status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        int i, total_bytes = 0;
        for (i = 0; i < ar_len; i++)
            total_bytes += iov_ar[i].ptr_array_len * iov_ar[i].bytes;
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_GETV, target, total_bytes);
    }
#   endif

    /* It isn't worth trying to optimize for the threshold here because
     * these operations aren't used much in GA. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_GetV_memcpy(iov_ar, ar_len);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_GetV_memcpy returned error\n");
    }
    else
    {
        status = OSPD_Allocate_handle(&osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Allocate_handle returned error\n");

        for (i=0; i<ar_len; i++)
        {
            for(j=0; j<iov_ar[i].ptr_ar_len; j++)
            {
                status = OSPD_NbGet(target,
                        iov_ar[i].source_ptr_ar[j],
                        iov_ar[i].target_ptr_ar[j],
                        iov_ar[i].size,
                        osp_handle);
                OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbGet returned with an error \n");
            }
        }

        status = OSPD_Wait_handle(osp_handle);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Wait_handle returned error\n");
    }

    fn_exit:
    if(target == my_rank && ospu_settings.network_bypass) OSPD_Release_handle(osp_handle);
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_NbGetV(int target,
        OSP_iov_t *iov_ar,
        int ar_len,
        OSP_handle_t osp_handle)
{
    int i, j, status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        int i, total_bytes = 0;
        for (i = 0; i < ar_len; i++)
            total_bytes += iov_ar[i].ptr_array_len * iov_ar[i].bytes;
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_GETV, target, total_bytes);
    }
#   endif

    /* It isn't worth trying to optimize for the threshold here because
     * these operations aren't used much in GA. */
    if (target == my_rank && ospu_settings.network_bypass)
    {
        status = OSPU_GetV_memcpy(iov_ar, ar_len);
        OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_GetV_memcpy returned error\n");
    }
    else
    {
        for (i=0; i<ar_len; i++)
        {
            for(j=0; j<iov_ar[i].ptr_ar_len; j++)
            {
                status = OSPD_NbGet(target,
                        iov_ar[i].source_ptr_ar[j],
                        iov_ar[i].target_ptr_ar[j],
                        iov_ar[i].size,
                        osp_handle);
                OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbGet returned with an error \n");
            }
        }
    }

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#endif /* OSPD_IMPLEMENTS_GETV */
