/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Get(int target, void* src, void* dst, int bytes)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_GET, target, bytes);
    }
#   endif

    if(target == my_rank && (bytes < ospu_settings.network_bypass_upper_limit_1d) )
    {
       status = OSPU_Get_memcpy(src, dst, bytes);
       OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Get_memcpy returned an error\n");
    }
    else
    {
        status = OSPD_Get(target, src, dst, bytes);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Get returned an error\n");
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}


int OSP_NbGet(int target, void* src, void* dst, int bytes, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    int my_rank = OSPD_Process_id(OSP_GROUP_WORLD);

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
        TAU_TRACE_SENDMSG (OSP_TAU_TAG_NBGET, target, bytes);
    }
#   endif

    if(target == my_rank && (bytes < ospu_settings.network_bypass_upper_limit_1d) )
    {
       status = OSPU_Get_memcpy(src, dst, bytes);
       OSPU_ERR_POP(status != OSP_SUCCESS, "OSPU_Get_memcpy returned an error\n");
    }
    else
    {
        status = OSPD_NbGet(target, src, dst, bytes, osp_handle);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_NbGet returned an error\n");
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
