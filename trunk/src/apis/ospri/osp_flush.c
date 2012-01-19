/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Flush(int proc) 
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Flush(proc); 
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Flush returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
