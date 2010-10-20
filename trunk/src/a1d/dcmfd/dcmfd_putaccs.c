/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_Packed_putaccs_protocol;

void OSPDI_RecvDone_packedputaccs_callback(void *clientdata, DCMF_Error_t *error)
{
    OSPD_Request_t *ospd_request = (OSPD_Request_t *) clientdata;
    OSPD_Buffer_t *ospd_buffer = ospd_request->ospd_buffer_ptr;
    OSPD_Packed_putaccs_header_t *header = (OSPD_Packed_putaccs_header_t *) ospd_buffer->buffer_ptr;
    int complete = 0;

    OSPDI_Unpack_strided_acc((void *) ((size_t) ospd_buffer->buffer_ptr + sizeof(OSPD_Packed_putaccs_header_t)),
                            header->data_size,
                            header->stride_level,
                            header->block_sizes,
                            header->target_ptr,
                            header->trg_stride_ar,
                            header->block_idx,
                            header->datatype,
                            (void *) &(header->scaling),
                            &complete);

    OSPDI_Release_request(ospd_request);
}

DCMF_Request_t* OSPDI_RecvSend_packedputaccs_callback(void *clientdata,
                                                     const DCQuad *msginfo,
                                                     unsigned count, /* TODO: this is not used */
                                                     size_t peer,
                                                     size_t sndlen,
                                                     size_t *rcvlen,
                                                     char **rcvbuf,
                                                     DCMF_Callback_t *cb_done)
{
    int status = 0;
    OSPD_Request_t *ospd_request = NULL;
    OSPD_Buffer_t *ospd_buffer = NULL;

    ospd_request = OSPDI_Get_request(0);
    OSPU_ERR_ABORT(status = (ospd_request == NULL),
                "OSPDI_Get_request returned NULL in OSPDI_RecvSend_packedputaccs_callback\n");

    ospd_buffer = OSPDI_Get_buffer(sndlen, 0);
    OSPU_ERR_ABORT(status = (ospd_buffer == NULL),
                "OSPDI_Get_buffer returned NULL in OSPDI_RecvSend_packedputaccs_callback\n");   

    *rcvlen = sndlen;
    *rcvbuf = ospd_buffer->buffer_ptr;

    ospd_request->ospd_buffer_ptr = ospd_buffer;
    cb_done->function = OSPDI_RecvDone_packedputaccs_callback;
    cb_done->clientdata = (void *) ospd_request;

    return &(ospd_request->request);
}

void OSPDI_RecvSendShort_packedputaccs_callback(void *clientdata,
                                               const DCQuad *msginfo,
                                               unsigned count, /* TODO: this is not used */
                                               size_t peer,
                                               const char *src,
                                               size_t bytes)
{
    OSPD_Packed_putaccs_header_t *header;
    void *packet_ptr = (void *) src;
    int complete = 0;

    header = (OSPD_Packed_putaccs_header_t *) packet_ptr;

    OSPDI_Unpack_strided_acc((void *) ((size_t)packet_ptr + sizeof(OSPD_Packed_putaccs_header_t)),
                            header->data_size,
                            header->stride_level,
                            header->block_sizes,
                            header->target_ptr,
                            header->trg_stride_ar,
                            header->block_idx,
                            header->datatype,
                            (void *) &(header->scaling),
                            &complete);
}

int OSPDI_Packed_putaccs_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_Send_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_SEND_PROTOCOL;
    conf.network = DCMF_TORUS_NETWORK;
    conf.cb_recv_short = OSPDI_RecvSendShort_packedputaccs_callback;
    conf.cb_recv_short_clientdata = NULL;
    conf.cb_recv = OSPDI_RecvSend_packedputaccs_callback;
    conf.cb_recv_clientdata = NULL;

    status = DCMF_Send_register(&OSPD_Packed_putaccs_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Send_register returned with error %d \n",
                status);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int OSPDI_Packed_putaccs(int target,
                        int stride_level,
                        int *block_sizes,
                        void* source_ptr,
                        int *src_stride_ar,
                        void* target_ptr,
                        int *trg_stride_ar,
                        OSP_datatype_t osp_type,
                        void *scaling)
{

    int status = OSP_SUCCESS;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *ospd_request = NULL;
    OSPD_Buffer_t *ospd_buffer = NULL;
    OSPD_Packed_putaccs_header_t header;
    void *packet_ptr = NULL, *data_ptr = NULL;
    int packet_size, data_size = 0, data_limit = 0;
    int block_idx[OSPC_MAX_STRIDED_DIM];
    int complete = 0;

    OSPU_FUNC_ENTER();

    OSPDI_Memset(block_idx, 0, (stride_level + 1) * sizeof(int));

    header.stride_level = stride_level;
    OSPDI_Memcpy(header.trg_stride_ar, trg_stride_ar, stride_level * sizeof(int));
    OSPDI_Memcpy(header.block_sizes, block_sizes, (stride_level + 1) * sizeof(int));
    header.datatype = osp_type;
    switch (osp_type)
    {
        case OSP_DOUBLE:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(double));
            break;
        case OSP_INT32:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(int32_t));
            break;
        case OSP_INT64:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(int64_t));
            break;
        case OSP_UINT32:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(uint32_t));
            break;
        case OSP_UINT64:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(uint64_t));
            break;
        case OSP_FLOAT:
            OSPDI_Memcpy((void *) &(header.scaling), scaling, sizeof(float));
            break;
        default:
            OSPU_ERR_POP(status, "Invalid osp_type received in Putacc operation \n");
            break;
    }

    while(!complete)
    {
       header.target_ptr = target_ptr;
       OSPDI_Memcpy(header.block_idx, block_idx, (stride_level + 1) * sizeof(int));

       /*Fetching buffer from the pool*/
       ospd_buffer = OSPDI_Get_buffer(ospd_settings.putacc_packetsize, 1);
       OSPU_ERR_ABORT(status = (ospd_buffer == NULL),
                               "OSPDI_Get_buffer returned with NULL\n");

       packet_ptr = ospd_buffer->buffer_ptr;

       data_ptr = (void *) ((size_t) packet_ptr 
              + sizeof(OSPD_Packed_putaccs_header_t));
       data_limit = ospd_settings.putacc_packetsize 
              - sizeof(OSPD_Packed_putaccs_header_t);
       data_size = 0;

       /*The packing function can modify the source ptr, target ptr, and block index*/
       OSPDI_Pack_strided(data_ptr,
                         data_limit,
                         stride_level,
                         block_sizes,
                         &source_ptr,
                         src_stride_ar,
                         &target_ptr,
                         trg_stride_ar,
                         block_idx,
                         &data_size,
                         &complete);

       /*Setting data size information in the header and copying it into the packet*/
       header.data_size = data_size;
       OSPDI_Memcpy((void *) packet_ptr, (void *) &header, sizeof(OSPD_Packed_putaccs_header_t));

       packet_size = data_size + sizeof(OSPD_Packed_putaccs_header_t);

       ospd_request = OSPDI_Get_request(1);
       OSPU_ERR_POP(status = (ospd_request == NULL),
              "OSPDI_Get_request returned error\n");
       ospd_request->ospd_buffer_ptr = ospd_buffer;

       done_callback.function = OSPDI_Request_done;
       done_callback.clientdata = (void *) ospd_request;

       status = DCMF_Send(&OSPD_Packed_putaccs_protocol,
                          &(ospd_request->request),
                          done_callback,
                          DCMF_SEQUENTIAL_CONSISTENCY,
                          target,
                          packet_size,
                          packet_ptr,
                          NULL,
                          0);
       OSPU_ERR_POP(status != DCMF_SUCCESS, "Send returned with an error \n");

       OSPD_Connection_send_active[target]++;
    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;

}

int OSPDI_Direct_putaccs(int target,
                        int stride_level,
                        int *block_sizes,
                        void* source_ptr,
                        int *src_stride_ar,
                        void* target_ptr,
                        int *trg_stride_ar,
                        OSP_datatype_t osp_type,
                        void *scaling,
                        OSPD_Handle_t *ospd_handle)
{
    int i, j, status = OSP_SUCCESS;
    OSPD_Putacc_header_t header;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *ospd_request;
    int chunk_count=1;
    int *block_sizes_w;
    int y=0;

    OSPU_FUNC_ENTER();

    status = OSPDI_Malloc((void **) &block_sizes_w, sizeof(int)*(stride_level+1));
    OSPU_ERR_POP(status != 0,
             "OSPDI_Malloc returned error in OSPDI_Direct_putaccs");

    OSPDI_Memcpy(block_sizes_w, block_sizes, sizeof(int)*(stride_level+1));

    for(i=1; i<=stride_level; i++)
        chunk_count = block_sizes[i]*chunk_count;

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
            OSPU_ERR_POP((status != OSP_SUCCESS),
                        "Invalid data type in putacc \n");
            break;
    }

    for(i=0; i<chunk_count; i++)
    {

        ospd_request = OSPDI_Get_request(1);
        OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error. \n");
        OSPDI_Set_handle(ospd_request, ospd_handle);

        ospd_handle->active++;

        done_callback.function = OSPDI_Request_done;
        done_callback.clientdata = (void *) ospd_request;

        header.target_ptr = target_ptr;

        status = DCMF_Send(&OSPD_Generic_putacc_protocol,
                           &(ospd_request->request),
                           done_callback,
                           DCMF_SEQUENTIAL_CONSISTENCY,
                           target,
                           block_sizes[0],
                           source_ptr,
                           (DCQuad *) &header,
                           (unsigned) 2);
        OSPU_ERR_POP((status != DCMF_SUCCESS), "DCMF Send returned with an error \n");

        OSPD_Connection_send_active[target]++;

        block_sizes_w[1]--;
        if(block_sizes_w[1]==0)
        {
               y=1;
               while(block_sizes_w[y] == 0)
               {
                  if(y == stride_level)
                  {
                     OSPU_ASSERT(i == chunk_count-1, status);
                     return status;
                  }
                  y++;
               }
               block_sizes_w[y]--;

               /*The strides done on lower dimensions should be subtracted as these are
                 included in the stride along the current dimension*/
               source_ptr = (void *) ((size_t) source_ptr 
                      + src_stride_ar[y-1] 
                      - (block_sizes[y-1] - 1) * src_stride_ar[y-2]);
               target_ptr = (void *) ((size_t) target_ptr 
                      + trg_stride_ar[y-1] 
                      - (block_sizes[y-1] - 1) * trg_stride_ar[y-2]);

               y--;
               while(y >= 1)
               {
                  block_sizes_w[y] = block_sizes[y];
                  y--;
               }
        }
        else
        {
               source_ptr = (void *) ((size_t) source_ptr + src_stride_ar[0]);
               target_ptr = (void *) ((size_t) target_ptr + trg_stride_ar[0]);
        }

    }

  fn_exit:
    OSPU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

int OSPDI_Recursive_putaccs(int target,
                        int stride_level,
                        int *block_sizes,
                        void* source_ptr,
                        int *src_stride_ar,
                        void* target_ptr,
                        int *trg_stride_ar,
                        OSP_datatype_t osp_type,
                        void *scaling,
                        OSPD_Handle_t *ospd_handle)
{
    int i, status = OSP_SUCCESS;
    OSPD_Putacc_header_t header;
    DCMF_Callback_t done_callback;
    OSPD_Request_t *ospd_request;

    OSPU_FUNC_ENTER();

    if (stride_level > 0)
    {

        for (i = 0; i < block_sizes[stride_level]; i++)
        {
            status = OSPDI_Recursive_putaccs(target,
                                         stride_level - 1,
                                         block_sizes,
                                         (void *) ((size_t) source_ptr + i * src_stride_ar[stride_level - 1]),
                                         src_stride_ar,
                                         (void *) ((size_t) target_ptr + i * trg_stride_ar[stride_level - 1]),
                                         trg_stride_ar,
                                         osp_type,
                                         scaling, 
                                         ospd_handle);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                    "OSPDI_Recursive_putaccs returned error in OSPDI_Direct_putaccs \n"); 

        }

    }
    else
    {

        ospd_request = OSPDI_Get_request(1);
        OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error. \n");
        OSPDI_Set_handle(ospd_request, ospd_handle);

        done_callback.function = OSPDI_Request_done;
        done_callback.clientdata = (void *) ospd_request;

        ospd_handle->active++;

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
                OSPU_ERR_POP((status != OSP_SUCCESS),"Invalid data type in putacc\n");
                break;
        }

        status = DCMF_Send(&OSPD_Generic_putacc_protocol,
                           &(ospd_request->request),
                           done_callback,
                           DCMF_SEQUENTIAL_CONSISTENCY,
                           target,
                           block_sizes[0],
                           source_ptr,
                           (DCQuad *) &header,
                           (unsigned) 2);
        OSPU_ERR_POP((status != DCMF_SUCCESS), "Putacc returned with an error \n");

        OSPD_Connection_send_active[target]++;

    }

  fn_exit: 
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_PutAccS(int target,
                int stride_level,
                int *block_sizes,
                void* source_ptr,
                int *src_stride_ar,
                void* target_ptr,
                int *trg_stride_ar,
                OSP_datatype_t osp_type,
                void* scaling)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle = NULL;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (block_sizes[0] > ospd_settings.putacc_packing_limit)
    {

        ospd_handle = OSPDI_Get_handle();
        OSPU_ERR_POP(status = (ospd_handle == NULL),
                "OSPDI_Get_handle returned NULL in OSPD_PutAccS\n");

        status = OSPDI_Direct_putaccs(target,
                                     stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar,
                                     osp_type,
                                     scaling,
                                     ospd_handle);
        OSPU_ERR_POP(status, "Direct putaccs function returned with an error \n");

        OSPDI_Conditional_advance(ospd_handle->active > 0);

        OSPDI_Release_handle(ospd_handle);

    }
    else
    {

        status = OSPDI_Packed_putaccs(target,
                                     stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar,
                                     osp_type,
                                     scaling);
        OSPU_ERR_POP(status, "Packed puts function returned with an error \n");

    }

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}

int OSPD_NbPutAccS(int target,
                int stride_level,
                int *block_sizes,
                void* source_ptr,
                int *src_stride_ar,
                void* target_ptr,
                int *trg_stride_ar,
                OSP_datatype_t osp_type,
                void* scaling,
                OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    ospd_handle = (OSPD_Handle_t *) osp_handle;

    if (block_sizes[0] > ospd_settings.putacc_packing_limit)
    {

        status = OSPDI_Direct_putaccs(target,
                                     stride_level,
                                     block_sizes,
                                     source_ptr,
                                     src_stride_ar,
                                     target_ptr,
                                     trg_stride_ar,
                                     osp_type,
                                     scaling,
                                     ospd_handle);
        OSPU_ERR_POP(status, "Direct putaccs function returned with an error \n");

    }
    else
    {

        if(ospd_settings.use_handoff)
        {
           OSPD_Op_handoff *op_handoff;
           status = OSPDI_Malloc((void **) &op_handoff, sizeof(OSPD_Op_handoff));
           OSPU_ERR_POP(status != 0, "OSPDI_Malloc returned with an error\n");           

           op_handoff->op_type = OSPD_PACKED_PUTACCS; 
           op_handoff->op.putaccs_op.target = target;
           op_handoff->op.putaccs_op.stride_level = stride_level;
           op_handoff->op.putaccs_op.block_sizes = block_sizes;
           op_handoff->op.putaccs_op.source_ptr = source_ptr;
           op_handoff->op.putaccs_op.src_stride_ar = src_stride_ar;
           op_handoff->op.putaccs_op.target_ptr = target_ptr;
           op_handoff->op.putaccs_op.trg_stride_ar = trg_stride_ar;
           op_handoff->op.putaccs_op.datatype = osp_type;
           op_handoff->op.putaccs_op.scaling = scaling;
           op_handoff->op.putaccs_op.ospd_handle = ospd_handle;

           ospd_handle->active++;

           if(OSPD_Op_handoff_queuetail == NULL)
           {
              OSPD_Op_handoff_queuehead = op_handoff;
              OSPD_Op_handoff_queuetail = op_handoff;
           }
           else
           {
             OSPD_Op_handoff_queuetail->next = op_handoff;
             OSPD_Op_handoff_queuetail = op_handoff;
           }
           op_handoff->next = NULL;

        }
        else
        {        

            status = OSPDI_Packed_putaccs(target,
                                         stride_level,
                                         block_sizes,
                                         source_ptr,
                                         src_stride_ar,
                                         target_ptr,
                                         trg_stride_ar,
                                         osp_type,
                                         scaling);
            OSPU_ERR_POP(status, "Packed puts function returned with an error \n");
  
        } 

    }

  fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

  fn_fail: 
    goto fn_exit;
}
