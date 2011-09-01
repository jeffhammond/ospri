/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfd_all.h"

DCMF_Protocol_t OSPD_Put_protocol;

/** @file osp.h.in */

/*! \addtogroup osp-dcmfd OSP DCMF Device Private Interface
 * @{
 */

/**
 * \brief Initialize DCMF put protocol
 *
 * \see OSPD_Put_rc
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPDI_Put_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Put_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_PUT_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;

    status = DCMF_Put_register(&OSPD_Put_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Put_register failed");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

/**
 * \brief DCMF device remote-completion-blocking put implementation
 *
 * \see OSPD_Put_lc, OSPD_Put_nb
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPDI_Put_end2end(ospd_window_t * window,
                      int target_rank, /* rank relative to window NOT world */
                      int target_window_offset,
                      DCMF_Memregion_t * source_mr,
                      int bytes)
{
    int status = OSP_SUCCESS;

    DCMF_Request_t request;

    DCMF_Callback_t remote_completion_cb;

    volatile int active;

    OSPU_FUNC_ENTER();

    remote_completion_cb.function = OSPDI_Generic_done;
    remote_completion_cb.clientdata = (void *) &active;

    active = 1;

    status = DCMF_Put(&OSPD_Put_protocol,
                      &request,
                      OSPD_Nocallback,
                      DCMF_RELAXED_CONSISTENCY,
                      target_rank,
                      bytes,
                      source_mr,
                      window->memregions[target_rank],
                      0, /* no offset because source memregion registered for each call */
                      target_window_offset,
                      remote_completion_cb);
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Put returned with an error");

    while (active > 0) DCMF_Messager_advance(0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

/**
 * \brief DCMF device local-completion-blocking put implementation
 *
 * \see OSPD_Put_rc, OSPD_Put_nb
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPDI_Put_bulk(ospd_window_t * window,
                   int target_rank, /* rank relative to window NOT world */
                   int target_window_offset,
                   DCMF_Memregion_t * source_mr,
                   int bytes)
{
    int status = OSP_SUCCESS;

    /* okay to put this request on stack since only non-NULL cb will be invoked before returning */
    DCMF_Request_t request;

    DCMF_Callback_t local_completion_cb;

    volatile int active;

    OSPU_FUNC_ENTER();

    local_completion_cb.function = OSPDI_Generic_done;
    local_completion_cb.clientdata = (void *) &active;

    active = 1;

    status = DCMF_Put(&OSPD_Put_protocol,
                      &request,
                      local_completion_cb,
                      DCMF_SEQUENTIAL_CONSISTENCY,
                      target_rank,
                      bytes,
                      source_mr,
                      window->memregions[target_rank],
                      0, /* no offset because source memregion registered for each call */
                      target_window_offset,
                      OSPD_Nocallback);
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Put returned with an error");

    while (active > 0) DCMF_Messager_advance(0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

/**
 * \brief DCMF device local-completion-blocking put implementation
 *
 * \see OSPD_Put_bulk, OSPD_iPut_handle
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPDI_Put_handle(ospd_window_t * window,
                     int target_rank, /* rank relative to window NOT world */
                     int target_window_offset,
                     DCMF_Memregion_t * source_mr,
                     int bytes,
                     ospd_handle_t * handle)
{
    int status = OSP_SUCCESS;

    DCMF_Callback_t local_completion_cb;
    DCMF_Callback_t remote_completion_cb;

    volatile int local_active;

    OSPU_FUNC_ENTER();

    local_completion_cb.function = OSPDI_Generic_done;
    local_completion_cb.clientdata = (void *) &local_active; /* use local data instead of this struct member in case this is faster */

    remote_completion_cb.function = OSPDI_Generic_done;
    remote_completion_cb.clientdata = (void *) (handle->active);

    local_active = 1;
    handle->active = 1;

    status = DCMF_Put(&OSPD_Put_protocol,
                      &(handle->request), /* must keep request in handle; cannot be deallocated until remote_cb is invoked */
                      local_completion_cb,
                      DCMF_RELAXED_CONSISTENCY, /* DCMF will drop into sequential internally */
                      target_rank,
                      bytes,
                      source_mr,
                      window->memregions[target_rank],
                      0, /* no offset because source memregion registered for each call */
                      target_window_offset,
                      remote_completion_cb);
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Put returned with an error");

    while (local_active > 0) DCMF_Messager_advance(0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

/**
 * \brief DCMF device remote-completion-blocking put implementation
 *
 * \see OSPD_Put_lc, OSPD_Put_nb
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPD_Put(ospd_window_t * window,
             unsigned target_rank, /* rank relative to window NOT world */
             unsigned target_window_offset,
             void * source_ptr,
             unsigned bytes,
             ospd_handle_t * handle,
             ospd_put_hint_t hint)
{
    int status = OSP_SUCCESS;
    unsigned bytes_out = 0;

    DCMF_Memregion_t source_mr;

    OSPU_FUNC_ENTER();

    DCMF_CriticalSection_enter(0);

    status  = DCMF_Memregion_create(&source_mr,
                                    &bytes_out,
                                    bytes,
                                    source_ptr,
                                    0);
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Memregion_create returned with an error");
    OSPU_ERR_POP(  bytes != bytes_out ,    "DCMF_Memregion_create failed to register all bytes");

    if ( ( hint == OSPD_PUT_HANDLE ) && (handle != NULL) )
    {
        status = OSPDI_Put_handle(window, target_rank, target_window_offset, &source_mr, bytes, handle);
        OSPU_ERR_POP( status != OSP_SUCCESS , "OSPDI_Put_handle returned with an error");
    }
    else if ( ( hint == OSPD_PUT_BULK ) && ( window->bulk_remote_completion_available == 1 ) )
    {
        status = OSPDI_Put_bulk(window, target_rank, target_window_offset, &source_mr, bytes);
        OSPU_ERR_POP( status != OSP_SUCCESS , "OSPDI_Put_bulk returned with an error");
    }
    else /* if all else fails, use OSPD_PUT_END2END */
    {
        status = OSPDI_Put_end2end(window, target_rank, target_window_offset, &source_mr, bytes);
        OSPU_ERR_POP( status != OSP_SUCCESS , "OSPDI_Put_end2end returned with an error");
    }

    fn_exit:
    DCMF_CriticalSection_exit(0);
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}


/*! @} */
