/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Process_id(OSP_group_t* group)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Process_id(group);

  fn_fail: 
    goto fn_exit;
}

int OSP_Process_total(OSP_group_t* group)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Process_total(group);

  fn_fail: 
    goto fn_exit;
}

int OSP_Node_id(OSP_group_t* group)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Node_id(group);

  fn_fail: 
    goto fn_exit;
}

int OSP_Node_total(OSP_group_t* group)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Node_total(group);

  fn_fail: 
    goto fn_exit;
}

double OSP_Time_seconds(void)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Time_seconds();

  fn_fail: 
    goto fn_exit;
}

unsigned long long OSP_Time_cycles(void)
{
    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

  fn_exit: 
    OSPU_FUNC_EXIT();
    return OSPD_Time_cycles();

  fn_fail: 
    goto fn_exit;
}
