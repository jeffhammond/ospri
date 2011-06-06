/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "pthreaddimpl.h"

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

int OSPD_Initialize(int thread_level)
{

    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

//    status = OSPDI_Read_parameters();
//    OSPU_ERR_POP(status != OSP_SUCCESS,
//                "OSPDI_Read_parameters returned with error \n");

    switch (thread_level)
    {
    case OSP_THREAD_SINGLE:
        ospd_settings.thread_safe = 0;
        break;
    case OSP_THREAD_FUNNELED:
        ospd_settings.thread_safe = 0;
        break;
    case OSP_THREAD_SERIALIZED:
        ospd_settings.thread_safe = 0;
        break;
    case OSP_THREAD_MULTIPLE:
        ospd_settings.thread_safe = 1;
        break;
    default:
        OSPU_ERR_POP(OSP_ERROR,
                    "Unsupported thread level provided in OSPD_Initialize \n");
        break;
    }

    /* NOTE: Clearly, the Pthread device only supports a single process. */
    OSPD_Process_info.my_rank = 0;
    OSPD_Process_info.num_ranks = 1;

    OSPD_Process_info.my_node = 0;
    OSPD_Process_info.num_nodes = 1;

//    status = OSPDI_Print_parameters();
//    OSPU_ERR_POP(status != OSP_SUCCESS,
//                "OSPDI_Print_parameters returned with error \n");

    OSPDI_CRITICAL_ENTER();

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}
