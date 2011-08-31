/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfd_all.h"

double OSPD_Time_seconds()
{
    double time;

    OSPU_FUNC_ENTER();

    time = DCMF_Timer();

  fn_exit: 
    OSPU_FUNC_EXIT();
    return time;

  fn_fail: 
    goto fn_exit;
}

unsigned long long OSPD_Time_cycles()
{
    unsigned long long time;

    OSPU_FUNC_ENTER();

    time = DCMF_Timebase();

  fn_exit: 
    OSPU_FUNC_EXIT();
    return time;

  fn_fail: 
    goto fn_exit;
}
