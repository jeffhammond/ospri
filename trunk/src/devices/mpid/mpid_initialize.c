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

    switch (thread_level)
    {
    case OSP_THREAD_MULTIPLE:
        required = MPI_THREAD_MULTIPLE;
        break;

    case OSP_THREAD_SERIALIZED:
        required = MPI_THREAD_SERIALIZED;
        break;

    case OSP_THREAD_FUNNELED:
        required = MPI_THREAD_FUNNELED;
        break;

    case OSP_THREAD_SINGLE:
        required = MPI_THREAD_SINGLE;
        break;

    default:
        OSPU_ERR_POP(1, "Invalid choice for OSP_thread_level\n");

    }

    MPI_Init_thread(&zero, NULL, required, &provided);

    OSPU_ERR_POP(required != provided,
                "MPI cannot provide requested thread support.");

    OSPD_Messager_info.thread_level = thread_level;

    MPI_Comm_size(MPI_COMM_WORLD, &(OSPD_Process_info.num_ranks));
    MPI_Comm_rank(MPI_COMM_WORLD, &(OSPD_Process_info.my_rank));

    result = OSPDI_Read_parameters();
    OSPU_ERR_POP(result != OSP_SUCCESS, "OSPDI_Read_parameters failed");

    /* FIXME: Need to do stuff here! */

    fn_exit: OSPU_FUNC_EXIT();
    return result;

    fn_fail: goto fn_exit;
}

