/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Rmw(int target,
           void* source_ptr_in,
           void* source_ptr_out,
           void* target_ptr,
           int bytes,
           OSP_atomic_op_t op,
           OSP_datatype_t osp_type)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

#   ifdef OSP_TAU_PROFILING
    {
      TAU_TRACE_SENDMSG (OSP_TAU_TAG_RMW, target, bytes);
    }
#   endif

    status = OSPD_Rmw(target,
                     source_ptr_in,
                     source_ptr_out,
                     target_ptr,
                     bytes,
                     op,
                     osp_type);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Rmw returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
