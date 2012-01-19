/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Create_counter(MPI_Comm comm, osp_counter_t * counter)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Create_counter(comm, counter);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Create_counter failed");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_Destroy_counter(osp_counter_t * counter)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Destroy_counter(counter);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Destroy_counter failed");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSP_Incr_counter(osp_counter_t * counter,
                     long increment,
                     long* original)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Incr_counter(counter, increment, original);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Incr_counter failed");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}
