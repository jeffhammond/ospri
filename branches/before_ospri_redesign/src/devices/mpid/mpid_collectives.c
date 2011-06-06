/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

void OSPD_Barrier_group(OSP_group_t* group) {

    OSPU_FUNC_ENTER();

    if(group == OSP_GROUP_WORLD) {
        MPI_Barrier(MPI_COMM_WORLD);
        goto fn_exit;
    } else {
        OSPU_ERR_ABORT(1,"OSPD_Barrier_group not implement for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return;

  fn_fail:
    goto fn_exit;

}
