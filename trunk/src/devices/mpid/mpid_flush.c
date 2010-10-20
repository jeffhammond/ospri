/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpidimpl.h"

int OSPD_Flush(int proc)
{
    int result = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

  fn_exit:
    OSPU_FUNC_EXIT();
    return result;

  fn_fail:
    goto fn_exit;
}
