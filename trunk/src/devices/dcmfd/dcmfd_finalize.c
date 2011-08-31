/* *- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in toplevel directory.
 */

#include "dcmfd_all.h"

int OSPD_Finalize(void)
{
    OSPU_FUNC_ENTER();

    MPI_Comm_free(&OSP_COMM_WORLD);

    fn_exit: OSPU_FUNC_EXIT();
    return 0;

    fn_fail: goto fn_exit;
}

void OSPD_Abort(int error_code, char error_message[])
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(1,
            "User called OSP_ABORT with error code %d, error msg: %s Program terminating abnormally \n",
            error_code,
            error_message);

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

