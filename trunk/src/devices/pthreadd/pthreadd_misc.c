/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "pthreaddimpl.h"

int OSPD_Process_id(OSP_group_t* group)
{
    int id;

    OSPU_FUNC_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        id = OSPD_Process_info.my_rank;
        goto fn_exit;
    }
    else
    {
        id = -1;
        OSPU_ERR_ABORT(1, "OSPD_Process_id not implement for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return id;

  fn_fail:
    goto fn_exit;
}

int OSPD_Process_total(OSP_group_t* group)
{
    int total;

    OSPU_FUNC_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        total = OSPD_Process_info.num_ranks;
        goto fn_exit;
    }
    else
    {
        total = -1;
        OSPU_ERR_ABORT(1,
                      "OSPD_Process_total not implement for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return total;

  fn_fail:
    goto fn_exit;
}

int OSPD_Node_id(OSP_group_t* group)
{
    int id;

    OSPU_FUNC_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        id = OSPD_Process_info.my_node;
        goto fn_exit;
    }
    else
    {
        id = -1;
        OSPU_ERR_ABORT(1, "OSPD_Node_id not implement for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return id;

  fn_fail:
    goto fn_exit;
}

int OSPD_Node_total(OSP_group_t* group)
{
    int total;

    OSPU_FUNC_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        total = OSPD_Process_info.num_nodes;
        goto fn_exit;
    }
    else
    {
        total = -1;
        OSPU_ERR_ABORT(1, "OSPD_Node_total not implement for non-world groups!");
        goto fn_fail;
    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return total;

  fn_fail:
    goto fn_exit;
}

double OSPD_Time_seconds()
{
    OSPU_FUNC_ENTER();

    /* TODO: implement this function */

  fn_exit:
    OSPU_FUNC_EXIT();
    return 0.0;

  fn_fail:
    goto fn_exit;
}

unsigned long long OSPD_Time_cycles()
{
    OSPU_FUNC_ENTER();

    /* TODO: implement this function */

  fn_exit:
    OSPU_FUNC_EXIT();
    return 0;

  fn_fail:
    goto fn_exit;
}
