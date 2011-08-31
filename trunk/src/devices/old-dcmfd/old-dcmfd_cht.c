/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

_BGP_Atomic global_atomic;
LockBox_Mutex_t global_lbmutex;

pthread_t OSPDI_CHT_pthread;

OSPD_Op_handoff *OSPD_Op_handoff_queuehead = NULL;
OSPD_Op_handoff *OSPD_Op_handoff_queuetail = NULL;

volatile int OSPD_Inside_handoff;

void OSPDI_Handoff_progress()
{
    int status = OSP_SUCCESS;
    OSPD_Op_handoff *op_handoff;

    OSPU_FUNC_ENTER();

    OSPD_Inside_handoff = 1;

    if (OSPD_Op_handoff_queuehead)
    {
        op_handoff = OSPD_Op_handoff_queuehead;

        if (op_handoff->op_type == OSPD_PACKED_PUTS)
        {

            status = OSPDI_Packed_puts(op_handoff->op.puts_op.target,
                                      op_handoff->op.puts_op.stride_level,
                                      op_handoff->op.puts_op.block_sizes,
                                      op_handoff->op.puts_op.source_ptr,
                                      op_handoff->op.puts_op.src_stride_ar,
                                      op_handoff->op.puts_op.target_ptr,
                                      op_handoff->op.puts_op.trg_stride_ar);
            OSPU_ERR_ABORT(status,
                          "OSPDI_Packed_puts returned with an error handoff progress\n");

            op_handoff->op.puts_op.ospd_handle->active--;

        }
        else if (op_handoff->op_type == OSPD_PACKED_PUTACCS)
        {

            status = OSPDI_Packed_putaccs(op_handoff->op.putaccs_op.target,
                                         op_handoff->op.putaccs_op.stride_level,
                                         op_handoff->op.putaccs_op.block_sizes,
                                         op_handoff->op.putaccs_op.source_ptr,
                                         op_handoff->op.putaccs_op.src_stride_ar,
                                         op_handoff->op.putaccs_op.target_ptr,
                                         op_handoff->op.putaccs_op.trg_stride_ar,
                                         op_handoff->op.putaccs_op.datatype,
                                         op_handoff->op.putaccs_op.scaling);
            OSPU_ERR_ABORT(status,
                          "OSPDI_Packed_putaccs returned with an error handoff progress\n");

            op_handoff->op.putaccs_op.ospd_handle->active--;

        }
        else
        {
            OSPU_ERR_ABORT(status = OSP_ERROR,
                          "Invalid op encountered in handoff progress. \n");
        }

        if (OSPD_Op_handoff_queuehead == OSPD_Op_handoff_queuetail)
        {
            OSPD_Op_handoff_queuehead = NULL;
            OSPD_Op_handoff_queuetail = NULL;
        }
        else
        {
            OSPD_Op_handoff_queuehead = OSPD_Op_handoff_queuehead->next;
        }

        OSPDI_Free(op_handoff);
    }

    OSPD_Inside_handoff = 0;

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void *OSPDI_CHT_advance_lock(void * dummy)
{
    OSPDI_GLOBAL_LOCK_ACQUIRE();
    while (1)
    {
        DCMF_Messager_advance(0);
        if (ospd_settings.use_handoff && (OSPD_Inside_handoff==0))
        {
            OSPDI_Handoff_progress();
        }
        OSPDI_GLOBAL_LOCK_RELEASE();
        OSPDI_Wait_cycles(ospd_settings.cht_pause_cycles);
        OSPDI_GLOBAL_LOCK_ACQUIRE();
    }
    OSPDI_GLOBAL_LOCK_RELEASE();
}

