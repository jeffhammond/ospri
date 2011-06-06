/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Release_segments(OSP_group_t* group, void* ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Release_segments(group, ptr);
    OSPU_ERR_POP(status, "OSPD_Release_segments returned an error\n");

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSP_Free_segment(void* ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Free_segment(ptr);
    OSPU_ERR_POP(status, "OSPD_Free_segment returned an error\n");

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
