/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

int OSPD_Put(int target, void* src, void* dst, int bytes)
{
    int result = MPI_SUCCESS;
    size_t src_disp, dst_disp;
 
    OSPU_FUNC_ENTER();

//    MPI_Put
    OSPU_ERR_POP(result!=MPI_SUCCESS,"Put returned with an error \n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return result;

  fn_fail:
    goto fn_exit;
}
