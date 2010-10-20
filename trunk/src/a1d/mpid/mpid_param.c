/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpi2rmadimpl.h"

int OSPDI_Read_parameters() {

    int result = OSP_SUCCESS;
    char* value = NULL;

    OSPU_FUNC_ENTER();

//    if ((value = getenv("OSP_ALIGNMENT")) != NULL) {
//        osp_alignment = atoi(value);
//    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return result;

  fn_fail:
    goto fn_exit;
}
