/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

int OSPD_Rank()
{
    OSPU_FUNC_ENTER();

  fn_exit:
    OSPU_FUNC_EXIT();
    return OSPD_Process_info.my_rank;

  fn_fail:
    goto fn_exit;
}

int OSPD_Size()
{
    OSPU_FUNC_ENTER();

  fn_exit:
    OSPU_FUNC_EXIT();
    return OSPD_Process_info.num_ranks;

  fn_fail:
    goto fn_exit;
}

double OSPD_Time_seconds()
{
    double time;

  fn_exit:
    OSPU_FUNC_EXIT();
    return MPI_Wtime();

  fn_fail:
    goto fn_exit;
} 


unsigned long long OSPD_Time_cycles()
{
    OSPU_FUNC_ENTER();

    /* FIXME: implement this function using Kaz's ASM */

    OSPU_error_printf("OSPD_Time_cycles not implemented.\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return 0;

  fn_fail:
    goto fn_exit;
}
