/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Put(int target, void* src, void* dst, int bytes)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_PUT, target, bytes);
    }
#   endif

    if(target == my_rank && (bytes < ospu_settings.network_bypass_upper_limit_1d) )
    {
       status = OSPU_Put_memcpy(src, dst, bytes);
       OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
    }
    else
    {
        status = OSPD_Put(target, src, dst, bytes);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Put returned an error\n");
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSP_NbPut(int target, void* src, void* dst, int bytes, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBPUT, target, bytes);
    }
#   endif

    /* Not sure if what is the right strategy for bypass.  OSPU_*_memcpy are blocking
     * but the overhead of going into DCMF_Put is likely not worth the savings
     * from said call being non-blocking.  This is especially true under heavy load
     * since we have determined that DMA vs. memcpy turns over when the NIC is getting
     * hammered.
     */
    if(target == my_rank && (bytes < ospu_settings.network_bypass_upper_limit_1d) )
    {
       status = OSPU_Put_memcpy(src, dst, bytes);
       OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Put_memcpy returned an error\n");
    }
    else
    {
        status = OSPD_NbPut(target, src, dst, bytes, osp_handle);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbPut returned an error\n");
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
