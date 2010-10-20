/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospd.h"
#include "ospu.h"

int OSP_Create_mutexes(OSP_group_t* group, 
                      int mutex_count, 
                      int *mutex_count_ar)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Create_mutexes(group,  
                                mutex_count, 
                                mutex_count_ar);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Create_mutexes returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}


int OSP_Destroy_mutexes(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Destroy_mutexes(group);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Destroy_mutexes returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Lock_mutex(OSP_group_t* group, 
                  int mutex, 
                  int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Lock_mutex(group, mutex, proc);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Lock_mutex returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Trylock_mutex(OSP_group_t* group, 
                     int mutex, 
                     int proc,
                     OSP_bool_t *acquired)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Trylock_mutex(group, mutex, proc, acquired);
    OSPU_ERR_POP(status, "OSPD_Lock_mutex returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSP_Unlock_mutex(OSP_group_t* group, 
                    int mutex, 
                    int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSPD_Unlock_mutex(group, mutex, proc);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Unlock_mutex returned an error\n");

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
