/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Finalize(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

    status = OSPD_Finalize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Finalize returned error\n");

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

void OSP_Abort(int error_code, char error_message[])
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

    OSPD_Abort(error_code,  error_message);

  fn_exit: 
    OSPU_FUNC_EXIT();
    return;

  fn_fail: 
    goto fn_exit;
}
