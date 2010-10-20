/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Initialize(int thread_level)
{
    int status = OSP_SUCCESS;
    static int osp_active = 0;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    /* we only want this function to run once */
    if(osp_active == 1)
    {
        return status;
    }
    osp_active = 1;

    status = OSPD_Initialize(thread_level);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Initialize returned error\n");

    status = OSPU_Read_parameters();
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_Read_parameters returned error\n");

    status = OSPU_Print_parameters();
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPU_Print_parameters returned error\n");

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
