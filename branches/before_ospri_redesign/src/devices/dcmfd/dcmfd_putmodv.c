/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Generic_putmod_protocol;

void OSPDI_RecvDone_putmod_callback(void *clientdata, DCMF_Error_t *error)
{
    int status = OSP_SUCCESS;

    OSPD_Request_t *ospd_request = (OSPD_Request_t *) clientdata;
    OSPD_Putmod_header_t *header = (OSPD_Putmod_header_t *) ospd_request->buffer_ptr;
    void* source_ptr = (void *) ((size_t) ospd_request->buffer_ptr + sizeof(OSPD_Putmod_header_t));
    int data_size = ospd_request->buffer_size - sizeof(OSPD_Putmod_header_t);

    switch (header->op)
    {
        case OSP_BXOR:
            switch (header->datatype)
            {
                case OSP_INT32:
                    OSPDI_MOD_BXOR(int32_t,
                            source_ptr,
                            header->target_ptr,
                            data_size / sizeof(int32_t));
                    break;
                case OSP_INT64:
                    OSPDI_MOD_BXOR(int64_t,
                            source_ptr,
                            header->target_ptr,
                            data_size / sizeof(int64_t));
                    break;
                case OSP_UINT32:
                    OSPDI_MOD_BXOR(uint32_t,
                            source_ptr,
                            header->target_ptr,
                            data_size / sizeof(uint32_t));
                    break;
                case OSP_UINT64:
                    OSPDI_MOD_BXOR(uint64_t,
                            source_ptr,
                            header->target_ptr,
                            data_size / sizeof(uint64_t));
                    break;
                default:
                    OSPU_ERR_ABORT(status, "Invalid datatype received in PutdodV\n");
                    break;
            }
            break;
        default:
            status = OSP_ERROR;
            OSPU_ERR_ABORT((status != OSP_SUCCESS), "Invalid op type in PutdodV\n");
            break;
    }

    OSPDI_Release_request(ospd_request);
}

DCMF_Request_t* OSPDI_RecvSend_putmod_callback(void *clientdata,
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
    OSPU_ERR_ABORT(status = (ospd_request == NULL),
            "OSPDI_Get_request returned NULL in OSPDI_RecvSend_putmod_callback\n");

    OSPU_ASSERT_ABORT(sizeof(OSPD_Putmod_header_t) == count*sizeof(DCQuad),
            "Header of invalid size received in OSPDI_RecvSend_putmod_callback\n")

    ospd_request->buffer_size = sndlen + sizeof(OSPD_Putmod_header_t);
    status = OSPDI_Malloc((void **) &(ospd_request->buffer_ptr),
                         sndlen + sizeof(OSPD_Putmod_header_t));
    OSPU_ERR_ABORT(status != 0,
            "OSPDI_Malloc failed in OSPDI_RecvSend_packedputs_callback\n");

    OSPDI_Memcpy(ospd_request->buffer_ptr,(void *) msginfo,sizeof(OSPD_Putmod_header_t));

    *rcvlen = sndlen;
    *rcvbuf = (char *) ((size_t) ospd_request->buffer_ptr + sizeof(OSPD_Putmod_header_t));

    cb_done->function = OSPDI_RecvDone_putmod_callback;
    cb_done->clientdata = (void *) ospd_request;

    return &(ospd_request->request);
}

void OSPDI_RecvSendShort_putmod_callback(void *clientdata,
                                        const DCQuad *msginfo,
                                        unsigned count,
                                        size_t peer,
                                        const char *src,
                                        size_t bytes)
{
    int status = OSP_SUCCESS;
    OSPD_Putmod_header_t *header = (OSPD_Putmod_header_t *) msginfo;
    void* source_ptr = (void *) src;

    switch (header->op)
    {
        case OSP_BXOR:
            switch (header->datatype)
            {
                case OSP_INT32:
                    OSPDI_MOD_BXOR(int32_t,
                            source_ptr,
                            header->target_ptr,
                            bytes / sizeof(int32_t));
                    break;
                case OSP_INT64:
                    OSPDI_MOD_BXOR(int64_t,
                            source_ptr,
                            header->target_ptr,
                            bytes / sizeof(int64_t));
                    break;
                case OSP_UINT32:
                    OSPDI_MOD_BXOR(uint32_t,
                            source_ptr,
                            header->target_ptr,
                            bytes / sizeof(uint32_t));
                    break;
                case OSP_UINT64:
                    OSPDI_MOD_BXOR(uint64_t,
                            source_ptr,
                            header->target_ptr,
                            bytes / sizeof(uint64_t));
                    break;
                default:
                    OSPU_ERR_ABORT(status, "Invalid datatype received in PutdodV\n");
                    break;
            }
            break;
        default:
            status = OSP_ERROR;
            OSPU_ERR_ABORT((status != OSP_SUCCESS), "Invalid op type in PutdodV\n");
            break;
    }
}

int OSPDI_Putmod_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Send_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_SEND_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    conf.cb_recv_short = OSPDI_RecvSendShort_putmod_callback;
    conf.cb_recv_short_clientdata = NULL;
    conf.cb_recv = OSPDI_RecvSend_putmod_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Send_register(&OSPD_Generic_putmod_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
            "DCMF_Send_register registration returned with error %d \n",
            status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}


int OSPDI_Direct_putmodv(int target,
                        OSP_iov_t *iov_ar,
                        int ar_len,
                        OSP_reduce_op_t osp_op,
                        OSP_datatype_t osp_type,
                        OSPD_Handle_t *ospd_handle)
{
    int i, j, status = OSP_SUCCESS;
    OSPD_Putmod_header_t header;
    OSPD_Request_t *ospd_request;
    DCMF_Callback_t done_callback;

    OSPU_FUNC_ENTER();

    header.datatype = osp_type;
    header.op = osp_op;

    for (i=0; i<ar_len; i++)
    {
        for(j=0; j<iov_ar[i].ptr_ar_len; j++)
        {
           ospd_request = OSPDI_Get_request(1);
           OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error.\n");
           OSPDI_Set_handle(ospd_request, ospd_handle);

           done_callback.function = OSPDI_Request_done;
           done_callback.clientdata = (void *) ospd_request;
 
           ospd_handle->active++;

           header.target_ptr = iov_ar[i].target_ptr_ar[j];
 
           status = DCMF_Send(&OSPD_Generic_putmod_protocol,
                              &(ospd_request->request),
                              done_callback,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              target,
                              iov_ar[i].size,
                              iov_ar[i].source_ptr_ar[j],
                              (DCQuad *) &header,
                              (unsigned) 2);
           OSPU_ERR_POP((status != DCMF_SUCCESS), "DCMF_Send returned with an error \n");
 
           OSPD_Connection_send_active[target]++;
        }
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_PutModV(int target,
                OSP_iov_t *iov_ar,
                int ar_len,
                OSP_reduce_op_t osp_op,
                OSP_datatype_t osp_type)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_PutModV.\n");

    status = OSPDI_Direct_putmodv(target,
                                 iov_ar,
                                 ar_len,
                                 osp_op,
                                 osp_type,
                                 ospd_handle);
    OSPU_ERR_POP(status, "OSPDI_Direct_putmodv returned with an error \n");

    OSPDI_Conditional_advance(ospd_handle->active > 0);

  fn_exit:
    OSPDI_Release_handle(ospd_handle);
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
