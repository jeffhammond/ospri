/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

osp_result_t OSP_Finalize(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Finalize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Finalize failed");

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

