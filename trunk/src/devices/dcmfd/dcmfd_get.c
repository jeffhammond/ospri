/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfd_all.h"

DCMF_Protocol_t OSPD_Get_protocol;

/** @file osp.h.in */

/*! \addtogroup osp-dcmfd OSP DCMF Device Private Interface
 * @{
 */

/**
 * \brief Initialize DCMF get protocol
 *
 * \see OSPD_Get
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */


int OSPDI_Get_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Get_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_GET_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;

    status = DCMF_Get_register(&OSPD_Get_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS, "DCMF_Get_register failed");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

/**
 * \brief DCMF device blocking get implementation
 *
 * \see OSPD_Get_nb
 *
 * \ingroup DEVICE_IMPLEMENTATION_DCMF
 */

int OSPDI_Get_end2end(ospd_window_t * window,
                      int source_rank, /* rank relative to window NOT world */
                      int source_window_offset,
                      DCMF_Memregion_t * target_mr,
                      int bytes)
{
    int status = OSP_SUCCESS;

    DCMF_Request_t request;

    DCMF_Callback_t completion_cb;

    volatile int active;

    OSPU_FUNC_ENTER();

    completion_cb.function = OSPDI_Generic_done;
    completion_cb.clientdata = (void *) &active;

    active = 1;

    status = DCMF_Get(&OSPD_Get_protocol,
                      &request,
                      completion_cb,
                      DCMF_RELAXED_CONSISTENCY,
                      source_rank,
                      bytes,
                      &(window->memregions[source_rank]),
                      target_mr,
                      source_window_offset,
                      0); /* no offset because source memregion registered for each call */
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Get returned with an error");

    while (active > 0) DCMF_Messager_advance(0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#if 0

int OSPDI_iGet_handle(ospd_window_t * window,
                      int source_rank, /* rank relative to window NOT world */
                      int source_window_offset,
                      DCMF_Memregion_t * target_mr,
                      int bytes,
                      ospd_handle_t * handle)
{
    int status = OSP_SUCCESS;

    DCMF_Callback_t completion_cb;

    OSPU_FUNC_ENTER();

    completion_cb.function = OSPDI_Generic_done;
    completion_cb.clientdata = (void *) &(handle->active);

    handle->active = 1;

    status = DCMF_Get(&OSPD_Get_protocol,
                      handle->request,
                      completion_cb,
                      DCMF_RELAXED_CONSISTENCY,
                      source_rank,
                      bytes,
                      window->memregions[source_rank],
                      &output_mr,
                      source_window_offset,
                      0); /* no offset because source memregion registered for each call */
    OSPU_ERR_POP( status != DCMF_SUCCESS , "DCMF_Get returned with an error");

    /* always poke once before returning */
    DCMF_Messager_advance(0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

#endif

/*! @} */
