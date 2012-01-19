/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpi.h"
#include "osp.h"
#include "ospd.h"
#include "ospu.h"

/* This is here because the build system does not yet have the necessary
 * logic to set these options for each device. */

osp_result_t OSP_Sync_group(OSP_group_t* group)
{
    osp_result_t status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Sync_group(group);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_Sync_group returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

osp_result_t OSP_NbSync_group(OSP_group_t* group, OSP_handle_t osp_handle)
{
    osp_result_t status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_NbSync_group(group, osp_handle);
    OSPU_ERR_POP(status!=OSP_SUCCESS, "OSPD_NbSync_group returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
