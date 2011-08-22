/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Rmw_protocol;
DCMF_Protocol_t OSPD_Rmw_response_protocol;

void OSPDI_RecvDone_rmw_response_callback(void *clientdata,
                                         DCMF_Error_t *error)
{
    int status = OSP_SUCCESS;
    OSPD_Request_t *ospd_request =  (OSPD_Request_t *) clientdata;
    OSPD_Buffer_t *ospd_buffer = ospd_request->ospd_buffer_ptr;
    OSPD_Rmw_response_header_t *header = (OSPD_Rmw_response_header_t *) ospd_buffer->buffer_ptr;
    OSPD_Handle_t *ospd_handle = (OSPD_Handle_t *) header->handle_ptr;
    void *reponse_data = (void *) (ospd_buffer->buffer_ptr + sizeof(OSPD_Rmw_response_header_t));
    void *source_ptr_out = header->source_ptr_out;

    OSPDI_Memcpy(source_ptr_out, reponse_data, header->bytes); 

    (ospd_handle->active)--;

    OSPDI_Release_request(ospd_request);
}

DCMF_Request_t* OSPDI_RecvSend_rmw_response_callback(void *clientdata,
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
    OSPD_Buffer_t *ospd_buffer;

    ospd_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (ospd_request == NULL),
                "OSPDI_Get_request returned NULL in OSPDI_RecvSend_rmw_response_callback.\n");

    ospd_buffer = OSPDI_Get_buffer(sndlen + sizeof(OSPD_Rmw_response_header_t), 0);
    OSPU_ERR_ABORT(status = (ospd_buffer == NULL),
                  "OSPDI_Get_buffer returned NULL.\n");

    OSPDI_Memcpy(ospd_buffer->buffer_ptr, msginfo, sizeof(DCQuad)*count);

    *rcvlen = sndlen;
    *rcvbuf = ospd_buffer->buffer_ptr + sizeof(OSPD_Rmw_response_header_t);

    ospd_request->ospd_buffer_ptr = ospd_buffer;
    cb_done->function = OSPDI_RecvDone_rmw_response_callback;
    cb_done->clientdata = (void *) ospd_request;

    return &(ospd_request->request);
}

void OSPDI_RecvSendShort_rmw_response_callback(void *clientdata,
                                              const DCQuad *msginfo,
                                              unsigned count,
                                              size_t peer,
                                              const char *src,
                                              size_t bytes)
{
    int status = OSP_SUCCESS;
    OSPD_Rmw_response_header_t *header = (OSPD_Rmw_response_header_t *) msginfo;
    OSPD_Handle_t *ospd_handle = (OSPD_Handle_t *) header->handle_ptr;
    void *response_data = (void *) src;
    void *source_ptr_out = header->source_ptr_out;
    
    OSPDI_Memcpy(source_ptr_out, response_data, header->bytes);

    (ospd_handle->active)--;

}

void OSPDI_RecvDone_rmw_callback(void *clientdata,
                                DCMF_Error_t *error)
{
    int status = OSP_SUCCESS;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *response_request = NULL;
    OSPD_Rmw_response_header_t response_header;
    OSPD_Request_t *ospd_request =  (OSPD_Request_t *) clientdata;
    OSPD_Buffer_t *ospd_buffer = ospd_request->ospd_buffer_ptr;
    OSPD_Rmw_header_t *header = (OSPD_Rmw_header_t *) ospd_buffer->buffer_ptr;
    OSP_datatype_t datatype = header->datatype;
    void *source = (void *) (ospd_buffer->buffer_ptr + sizeof(OSPD_Rmw_header_t));
    void *target = header->target_ptr;
    void *original = NULL;

    status = OSPDI_Malloc(&original, header->bytes);
    OSPU_ERR_ABORT(status,
                "OSPDI_Malloc returned error in OSPDI_RecvDone_rmw_callback\n"); 
 
    if(header->op == OSP_FETCH_AND_ADD)
    {
       likely_if(datatype == OSP_DOUBLE) 
       {
           OSPDI_FETCHANDADD_EXECUTE(double, source, target, original, header->bytes/sizeof(double));
       }
       else
       {
          switch (datatype)
          {
          case OSP_INT32:
              OSPDI_FETCHANDADD_EXECUTE(int32_t, source, target, original, header->bytes/sizeof(int32_t));
              break;
          case OSP_INT64:
              OSPDI_FETCHANDADD_EXECUTE(int64_t, source, target, original, header->bytes/sizeof(int64_t));
              break; 
          case OSP_UINT32:
              OSPDI_FETCHANDADD_EXECUTE(uint32_t, source, target, original, header->bytes/sizeof(uint32_t));
              break;
          case OSP_UINT64:
              OSPDI_FETCHANDADD_EXECUTE(uint64_t, source, target, original, header->bytes/sizeof(uint64_t));
              break;
          case OSP_FLOAT:
              OSPDI_FETCHANDADD_EXECUTE(float, source, target, original, header->bytes/sizeof(float));
              break;
          default:
              status = OSP_ERROR;
              OSPU_ERR_ABORT((status != OSP_SUCCESS), "Invalid data type in rmw \n");
              break;
          }
       }    
    }
    else if(header->op = OSP_SWAP)
    {
        OSPDI_Memcpy(original, target, header->bytes); 
        OSPDI_Memcpy(target, source, header->bytes); 
    }

    response_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (ospd_request == NULL), "OSPDI_Get_request returned NULL in RMW callback\n");

    response_request->buffer_ptr = original;
 
    response_header.bytes = header->bytes;
    response_header.source_ptr_out = header->source_ptr_out;
    response_header.handle_ptr = header->handle_ptr;

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) response_request;

    status = DCMF_Send(&OSPD_Rmw_response_protocol,
                       &(response_request->request),
                       done_callback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       header->source,
                       header->bytes,
                       original,
                       (DCQuad *) &response_header,
                       (unsigned) 1);
    OSPU_ERR_ABORT((status != DCMF_SUCCESS), "DCMF_Send returned with an error OSPD_Rmw\n"); 

    OSPDI_Release_request(ospd_request);
}

DCMF_Request_t* OSPDI_RecvSend_rmw_callback(void *clientdata,
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
    OSPD_Buffer_t *ospd_buffer;

    ospd_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (ospd_request == NULL),
                "OSPDI_Get_request returned NULL in OSPDI_RecvSend_rmw_callback.\n");

    ospd_buffer = OSPDI_Get_buffer(sndlen + sizeof(OSPD_Rmw_header_t), 0);
    OSPU_ERR_ABORT(status = (ospd_buffer == NULL),
                  "OSPDI_Get_buffer returned NULL.\n");

    OSPDI_Memcpy(ospd_buffer->buffer_ptr, msginfo, sizeof(DCQuad)*count);

    *rcvlen = sndlen;
    *rcvbuf = ospd_buffer->buffer_ptr + sizeof(OSPD_Rmw_header_t);

    ospd_request->ospd_buffer_ptr = ospd_buffer;
    cb_done->function = OSPDI_RecvDone_rmw_callback;
    cb_done->clientdata = (void *) ospd_request;

    return &(ospd_request->request);
}

void OSPDI_RecvSendShort_rmw_callback(void *clientdata,
                                     const DCQuad *msginfo,
                                     unsigned count,
                                     size_t peer,
                                     const char *src,
                                     size_t bytes)
{
    int status = OSP_SUCCESS;
    DCMF_Callback_t done_callback;
    OSPD_Rmw_header_t *header = (OSPD_Rmw_header_t *) msginfo;
    OSP_datatype_t datatype = header->datatype;
    void *source = (void *) src;
    void *target = header->target_ptr;
    void *original = NULL;
    OSPD_Request_t *response_request = NULL;
    OSPD_Rmw_response_header_t response_header;

    status = OSPDI_Malloc(&original, bytes);
    OSPU_ERR_ABORT(status,
                "OSPDI_Malloc returned error in OSPDI_RecvSendShort_rmw_callback\n"); 
 
    if(header->op == OSP_FETCH_AND_ADD)
    {
       likely_if(datatype == OSP_DOUBLE) 
       {
           OSPDI_FETCHANDADD_EXECUTE(double, source, target, original, header->bytes/sizeof(double));
       }
       else
       {
          switch (datatype)
          {
          case OSP_INT32:
              OSPDI_FETCHANDADD_EXECUTE(int32_t, source, target, original, header->bytes/sizeof(int32_t));
              break;
          case OSP_INT64:
              OSPDI_FETCHANDADD_EXECUTE(int64_t, source, target, original, header->bytes/sizeof(int64_t));
              break; 
          case OSP_UINT32:
              OSPDI_FETCHANDADD_EXECUTE(uint32_t, source, target, original, header->bytes/sizeof(uint32_t));
              break;
          case OSP_UINT64:
              OSPDI_FETCHANDADD_EXECUTE(uint64_t, source, target, original, header->bytes/sizeof(uint64_t));
              break;
          case OSP_FLOAT:
              OSPDI_FETCHANDADD_EXECUTE(float, source, target, original, header->bytes/sizeof(float));
              break;
          default:
              status = OSP_ERROR;
              OSPU_ERR_ABORT((status != OSP_SUCCESS), "Invalid data type in rmw \n");
              break;
          }
       }    
    }
    else if(header->op = OSP_SWAP)
    {
        OSPDI_Memcpy(original, target, header->bytes); 
        OSPDI_Memcpy(target, source, header->bytes); 
    }

    response_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (response_request == NULL), "OSPDI_Get_request returned NULL in RMW callback\n");

    response_request->buffer_ptr = original;
 
    response_header.bytes = header->bytes;
    response_header.source_ptr_out = header->source_ptr_out;
    response_header.handle_ptr = header->handle_ptr;

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) response_request;

    status = DCMF_Send(&OSPD_Rmw_response_protocol,
                       &(response_request->request),
                       done_callback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       header->source,
                       header->bytes,
                       original,
                       (DCQuad *) &response_header,
                       (unsigned) 1);
    OSPU_ERR_ABORT((status != DCMF_SUCCESS), "DCMF_Send returned with an error OSPD_Rmw\n"); 
    
}

int OSPDI_Rmw_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Send_Configuration_t conf;
    
    OSPU_FUNC_ENTER();
    
    conf.protocol = DCMF_DEFAULT_SEND_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    conf.cb_recv_short = OSPDI_RecvSendShort_rmw_callback;
    conf.cb_recv_short_clientdata = NULL; 
    conf.cb_recv = OSPDI_RecvSend_rmw_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Send_register(&OSPD_Rmw_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "rmw registartion returned with error %d \n",
                status);

    conf.cb_recv_short = OSPDI_RecvSendShort_rmw_response_callback;
    conf.cb_recv = OSPDI_RecvSend_rmw_response_callback;

    status = DCMF_Send_register(&OSPD_Rmw_response_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "rmw_response registartion returned with error %d \n",
                status);

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSPD_Rmw(int target,
            void* source_ptr_in,
            void* source_ptr_out,
            void* target_ptr,
            int bytes,
            OSP_atomic_op_t op,
            OSP_datatype_t osp_type)
{
    int status = OSP_SUCCESS;
    DCMF_Callback_t done_callback;
    DCMF_Request_t request;
    OSPD_Rmw_header_t header;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = OSPDI_Get_handle();
    OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_Rmw\n");

    header.source = OSPD_Process_info.my_rank; 
    header.source_ptr_out = source_ptr_out;
    header.target_ptr = target_ptr;
    header.bytes = bytes;
    header.op = op;
    header.datatype = osp_type;
    header.handle_ptr = ospd_handle;
 
    ospd_handle->active++;

    status = DCMF_Send(&OSPD_Rmw_protocol,
                       &request,
                       OSPD_Nocallback,
                       DCMF_SEQUENTIAL_CONSISTENCY,
                       target,
                       bytes,
                       source_ptr_in,
                       (DCQuad *) &header,
                       (unsigned) 2);
    OSPU_ERR_POP((status != DCMF_SUCCESS), "DCMF_Send returned with an error OSPD_Rmw\n");    

    OSPDI_Conditional_advance(ospd_handle->active > 0);

  fn_exit:
    OSPDI_Release_handle(ospd_handle);
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}
