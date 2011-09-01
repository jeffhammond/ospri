/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

int OSPD_Initialize(int thread_level)
{

    int result = MPI_SUCCESS;
    int required;
    int provided;
    const int zero = 0;

    OSPU_FUNC_ENTER();

    /* FIXME: Need to do stuff here! */

    fn_exit: OSPU_FUNC_EXIT();
    return result;

    fn_fail: goto fn_exit;
}

