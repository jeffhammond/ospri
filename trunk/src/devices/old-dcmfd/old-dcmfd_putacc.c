/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Generic_putacc_protocol;

void OSPDI_RecvDone_putacc_callback(void *clientdata, DCMF_Error_t *error)
{
    int status = OSP_SUCCESS;

    OSPD_Request_t *ospd_request = (OSPD_Request_t *) clientdata;
    OSPD_Putacc_header_t *header = (OSPD_Putacc_header_t *) ospd_request->buffer_ptr;
    void* source_ptr = (void *) ((size_t) ospd_request->buffer_ptr + sizeof(OSPD_Putacc_header_t));
    int data_size = ospd_request->buffer_size - sizeof(OSPD_Putacc_header_t);

    switch (header->datatype)
    {
        case OSP_DOUBLE:
            OSPDI_ACC(double, source_ptr, header->target_ptr, (header->scaling).double_value, data_size/sizeof(double));
            break;
        case OSP_INT32:
            OSPDI_ACC(int32_t, source_ptr, header->target_ptr, (header->scaling).int32_value, data_size / sizeof(int32_t));
            break;
        case OSP_INT64:
            OSPDI_ACC(int64_t, source_ptr, header->target_ptr, (header->scaling).int64_value, data_size / sizeof(int64_t));
            break;
        case OSP_UINT32:
            OSPDI_ACC(uint32_t, source_ptr, header->target_ptr, (header->scaling).uint32_value, data_size / sizeof(uint32_t));
            break;
        case OSP_UINT64:
            OSPDI_ACC(uint64_t, source_ptr, header->target_ptr, (header->scaling).uint64_value, data_size / sizeof(uint64_t));
            break;
        case OSP_FLOAT:
            OSPDI_ACC(float, source_ptr, header->target_ptr, (header->scaling).float_value, data_size / sizeof(float));
            break;
        default:
            OSPU_ERR_ABORT(status, "Invalid datatype received in Putacc operation\n");
            break;
    }

    OSPDI_Release_request(ospd_request);
}

DCMF_Request_t* OSPDI_RecvSend_putacc_callback(void *clientdata,
                                              const DCQuad *msginfo,
                                              unsigned count,
                                              size_t peer,
                                              size_t sndlen,
                                              size_t *rcvlen,
                                              char **rcvbuf,
                                              DCMF_Callback_t *cb_done)
{
    int status = 0;
    OSPD_Request_t *ospd_request;

    ospd_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (ospd_request == NULL), "OSPDI_Get_request returned NULL in OSPDI_RecvSend_putacc_callback\n");

    OSPU_ASSERT_ABORT(sizeof(OSPD_Putacc_header_t) == count*sizeof(DCQuad), "Header of invalid size received in OSPDI_RecvSend_putacc_callback\n")

    ospd_request->buffer_size = sndlen + sizeof(OSPD_Putacc_header_t);
    status = OSPDI_Malloc((void **) &(ospd_request->buffer_ptr), sndlen + sizeof(OSPD_Putacc_header_t));
    OSPU_ERR_ABORT(status != 0, "OSPDI_Malloc failed in OSPDI_RecvSend_packedputs_callback\n");

    OSPDI_Memcpy(ospd_request->buffer_ptr,(void *) msginfo,sizeof(OSPD_Putacc_header_t));

    *rcvlen = sndlen;
    *rcvbuf = (char *) ((size_t) ospd_request->buffer_ptr + sizeof(OSPD_Putacc_header_t));

    cb_done->function = OSPDI_RecvDone_putacc_callback;
    cb_done->clientdata = (void *) ospd_request;

    return &(ospd_request->request);
}

void OSPDI_RecvSendShort_putacc_callback(void *clientdata,
                                        const DCQuad *msginfo,
                                        unsigned count,
                                        size_t peer,
                                        const char *src,
                                        size_t bytes)
{
    int status = OSP_SUCCESS;
    OSPD_Putacc_header_t *header = (OSPD_Putacc_header_t *) msginfo;
    void* source_ptr = (void *) src;

    switch (header->datatype)
    {
        case OSP_DOUBLE:
            OSPDI_ACC(double, source_ptr, header->target_ptr, (header->scaling).double_value, bytes/sizeof(double));
            break;
        case OSP_INT32:
            OSPDI_ACC(int32_t, source_ptr, header->target_ptr, (header->scaling).int32_value, bytes / sizeof(int32_t));
            break;
        case OSP_INT64:
            OSPDI_ACC(int64_t, source_ptr, header->target_ptr, (header->scaling).int64_value, bytes / sizeof(int64_t));
            break;
        case OSP_UINT32:
            OSPDI_ACC(uint32_t, source_ptr, header->target_ptr, (header->scaling).uint32_value, bytes / sizeof(uint32_t));
            break;
        case OSP_UINT64:
            OSPDI_ACC(uint64_t, source_ptr, header->target_ptr, (header->scaling).uint64_value, bytes / sizeof(uint64_t));
            break;
        case OSP_FLOAT:
            OSPDI_ACC(float, source_ptr, header->target_ptr, (header->scaling).float_value, bytes/sizeof(float));
            break;
        default:
            OSPU_ERR_ABORT(status, "Invalid datatype received in Putacc operation \n") ;
            break;
    }
}

int OSPDI_Putacc_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Send_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_SEND_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    conf.cb_recv_short = OSPDI_RecvSendShort_putacc_callback;
    conf.cb_recv_short_clientdata = NULL;
    conf.cb_recv = OSPDI_RecvSend_putacc_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Send_register(&OSPD_Generic_putacc_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
            "DCMF_Send_register registration returned with error %d \n",
            status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_PutAcc(int target,
               void* source_ptr,
               void* target_ptr,
               int bytes,
               OSP_datatype_t osp_type,
               void* scaling)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t done_callback;
    volatile int active;
    OSPD_Putacc_header_t header;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    done_callback.function = OSPDI_Generic_done;
    done_callback.clientdata = (void *) &active;
    active = 1;

    header.target_ptr = target_ptr;
    header.datatype = osp_type;

    switch (osp_type)
    {
        case OSP_DOUBLE:
            (header.scaling).double_value = *((double *) scaling);
            break;
        case OSP_INT32:
            (header.scaling).int32_value = *((int32_t *) scaling);
            break;
        case OSP_INT64:
            (header.scaling).int64_value = *((int64_t *) scaling);
            break;
        case OSP_UINT32:
            (header.scaling).uint32_value = *((uint32_t *) scaling);
            break;
        case OSP_UINT64:
            (header.scaling).uint64_value = *((uint64_t *) scaling);
            break;
        case OSP_FLOAT:
            (header.scaling).float_value = *((float *) scaling);
            break;
        default:
            status = OSP_ERROR;
            OSPU_ERR_POP((status != OSP_SUCCESS), "Invalid data type in putacc \n");
            break;
    }

    status = DCMF_Send(&OSPD_Generic_putacc_protocol,
                       &request,
                       done_callback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       target,
                       bytes,
                       source_ptr,
                       (DCQuad *) &header,
                       (unsigned) 2);
    OSPU_ERR_POP((status != OSP_SUCCESS), "DCMF_Send returned with an error \n");

    OSPD_Connection_send_active[target]++;
    OSPDI_Conditional_advance(active > 0);

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPD_NbPutAcc(int target,
                 void* source_ptr,
                 void* target_ptr,
                 int bytes,
                 OSP_datatype_t osp_type,
                 void* scaling,
                 OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *ospd_request;
    OSPD_Putacc_header_t header;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    ospd_handle->active++;

    ospd_request = OSPDI_Get_request(1);
    OSPU_ERR_POP(status = (ospd_request == NULL), "OSPDI_Get_request returned NULL request \n");
    OSPDI_Set_handle(ospd_request, ospd_handle);

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) ospd_request;

    header.target_ptr = target_ptr;
    header.datatype = osp_type;

    switch (osp_type)
    {
        case OSP_DOUBLE:
            (header.scaling).double_value = *((double *) scaling);
            break;
        case OSP_INT32:
            (header.scaling).int32_value = *((int32_t *) scaling);
            break;
        case OSP_INT64:
            (header.scaling).int64_value = *((int64_t *) scaling);
            break;
        case OSP_UINT32:
            (header.scaling).uint32_value = *((uint32_t *) scaling);
            break;
        case OSP_UINT64:
            (header.scaling).uint64_value = *((uint64_t *) scaling);
            break;
        case OSP_FLOAT:
            (header.scaling).float_value = *((float *) scaling);
            break;
        default:
            status = OSP_ERROR;
            OSPU_ERR_POP(status, "Invalid data type in putacc \n");
            break;
    }

    status = DCMF_Send(&OSPD_Generic_putacc_protocol,
                       &(ospd_request->request),
                       done_callback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       target,
                       bytes,
                       source_ptr,
                       (DCQuad *) &header,
                       (unsigned) 2);
    OSPU_ERR_POP((status != DCMF_SUCCESS), "DCMF_Send returned with an error \n");

    OSPD_Connection_send_active[target]++;

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}
