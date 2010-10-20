/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "dcmfdimpl.h"

DCMF_Protocol_t OSPD_GlobalBarrier_protocol;
DCMF_Protocol_t OSPD_GlobalBcast_protocol;

DCMF_CollectiveProtocol_t OSPD_GlobalAllreduce_protocol;
DCMF_CollectiveProtocol_t OSPD_Barrier_protocol, OSPD_Localbarrier_protocol;
DCMF_CollectiveProtocol_t *barrier_ptr, *localbarrier_ptr;
DCMF_CollectiveProtocol_t *barrier_ptr, *localbarrier_ptr;
DCMF_Geometry_t geometry;
DCMF_Barrier_Configuration_t barrier_conf;
DCMF_Allreduce_Configuration_t allreduce_conf;
DCMF_CollectiveRequest_t crequest;
unsigned *allreduce_ranklist;

DCMF_Geometry_t *getGeometry (int x)
{
    return &geometry;
}

int OSPDI_GlobalBarrier_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_GlobalBarrier_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_GLOBALBARRIER_PROTOCOL;
    status = DCMF_GlobalBarrier_register(&OSPD_GlobalBarrier_protocol, &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_GlobalBarrier_register returned with error %d \n",
                status);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPDI_GlobalBarrier()
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t done_callback;
    volatile int active;

    OSPU_FUNC_ENTER();

    active = 1;
    done_callback.function = OSPDI_Generic_done;
    done_callback.clientdata = (void *) &active;

    status = DCMF_GlobalBarrier(&OSPD_GlobalBarrier_protocol,
                                &request,
                                done_callback);
    OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                  "DCMF_GlobalBarrier returned with an error");

    OSPDI_Conditional_advance(active > 0);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPD_Barrier_group(OSP_group_t* group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {

        status = OSPDI_GlobalBarrier();
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "DCMF_GlobalBarrier returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_Barrier_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPD_Sync_group(OSP_group_t* group)
{

    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_Flush_all();
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_Flush_all returned with an error");
        status = OSPDI_GlobalBarrier();
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_GlobalBarrier returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_Sync_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPDI_NbGlobalBarrier(OSPD_Handle_t *ospd_handle)
{

    int status = OSP_SUCCESS;
    OSPD_Request_t *ospd_request;
    DCMF_Callback_t done_callback;
    volatile int active;

    OSPU_FUNC_ENTER();

    ospd_request = OSPDI_Get_request(1);
    OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error \n");
    OSPDI_Set_handle(ospd_request, ospd_handle);

    ospd_handle->active++;

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) ospd_request;

    status = DCMF_GlobalBarrier(&OSPD_GlobalBarrier_protocol,
                                &(ospd_request->request),
                                done_callback);
    OSPU_ERR_ABORT(status != DCMF_SUCCESS,
                  "DCMF_GlobalBarrier returned with an error");

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPD_NbBarrier_group(OSP_group_t* group, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        ospd_handle = (OSPD_Handle_t *) osp_handle;

        status = OSPDI_NbGlobalBarrier(ospd_handle);
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "DCMF_NbGlobalBarrier returned with an error");

        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_NbBarrier_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPD_NbSync_group(OSP_group_t* group, OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        ospd_handle = (OSPD_Handle_t *) osp_handle;

        /*This has to be replace with a non-blocking flushall to make it truly non blocking*/
        status = OSPDI_Flush_all();
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_Flush_all returned with an error");

        status = OSPDI_NbGlobalBarrier(ospd_handle);
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_NbGlobalBarrier returned with an error");

        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1, "OSPD_NbSync_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPDI_GlobalAllreduce_initialize()
{
    int i,status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    barrier_conf.protocol = DCMF_GI_BARRIER_PROTOCOL;
    barrier_conf.cb_geometry = getGeometry;
    status = DCMF_Barrier_register(&OSPD_Barrier_protocol,
                                   &barrier_conf);

    barrier_conf.protocol = DCMF_LOCKBOX_BARRIER_PROTOCOL;
    barrier_conf.cb_geometry = getGeometry;
    status = DCMF_Barrier_register(&OSPD_Localbarrier_protocol,
                                   &barrier_conf);

    /*This has to eventually freed, not being done now*/
    status = OSPDI_Malloc((void **) &allreduce_ranklist, OSPD_Process_info.num_ranks * sizeof(unsigned));
    OSPU_ERR_POP(status != 0,
                "OSPDI_Malloc returned with error %d \n", status);

    for(i=0; i<OSPD_Process_info.num_ranks; i++)
        allreduce_ranklist[i] = i;

    barrier_ptr = &OSPD_Barrier_protocol;
    localbarrier_ptr  = &OSPD_Localbarrier_protocol;
    status = DCMF_Geometry_initialize(&geometry,
                                      0,
                                      allreduce_ranklist,
                                      OSPD_Process_info.num_ranks,
                                      &barrier_ptr,
                                      1,
                                      &localbarrier_ptr,
                                      1,
                                      &crequest,
                                      0,
                                      1);

    allreduce_conf.protocol = DCMF_TORUS_BINOMIAL_ALLREDUCE_PROTOCOL;
    allreduce_conf.cb_geometry = getGeometry;
    allreduce_conf.reuse_storage = 1;
    status = DCMF_Allreduce_register(&OSPD_GlobalAllreduce_protocol,
                                     &allreduce_conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_Allreduce_register returned with error %d \n", status);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPDI_GlobalAllreduce(int count,
                         OSP_reduce_op_t osp_op,
                         OSP_datatype_t osp_type,
                         void *in,
                         void *out)
{
    int status = OSP_SUCCESS;
    DCMF_CollectiveRequest_t ar_crequest;
    DCMF_Callback_t done_callback;
    DCMF_Op reduce_op;
    DCMF_Dt datatype;
    int bytes = 0;
    void *in_abs = NULL;
    volatile unsigned ga_active = 0;

    OSPU_FUNC_ENTER();

    switch (osp_op)
    {
        case OSP_SUM:
            reduce_op = DCMF_SUM;
            break;
        case OSP_PROD:
            reduce_op = DCMF_PROD;
            break;
        case OSP_MAX:
        case OSP_MAXABS:
            reduce_op = DCMF_MAX;
            break;
        case OSP_MIN:
        case OSP_MINABS:
            reduce_op = DCMF_MIN;
            break;
        case OSP_OR:
            reduce_op = DCMF_LOR;
            break;
        default:
            OSPU_ERR_POP(status != DCMF_SUCCESS, "Unsupported OSP_reduce_op \n");
            break;
    }

    if (osp_op == OSP_MAXABS || osp_op == OSP_MINABS)
    {
        switch (osp_type)
        {
        case OSP_DOUBLE:
            datatype = DCMF_DOUBLE;
            bytes = count * sizeof(double);
            status = OSPDI_Malloc(&in_abs, bytes);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPDI_Malloc returned error in OSPDI_GlobalAllreduce \n");
            OSPDI_ABS(double, in, in_abs, count);
            in = in_abs;
            break;
        case OSP_INT32:
            datatype = DCMF_SIGNED_INT;
            bytes = count * sizeof(int32_t);
            status = OSPDI_Malloc(&in_abs, bytes);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPDI_Malloc returned error in OSPDI_GlobalAllreduce \n");
            OSPDI_ABS(int32_t, in, in_abs, count);
            in = in_abs;
            break;
        case OSP_INT64:
            datatype = DCMF_SIGNED_LONG_LONG;
            bytes = count * sizeof(int64_t);
            status = OSPDI_Malloc(&in_abs, bytes);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPDI_Malloc returned error in OSPDI_GlobalAllreduce \n");
            OSPDI_ABS(int64_t, in, in_abs, count);
            in = in_abs;
            break;
        case OSP_UINT32:
            datatype = DCMF_UNSIGNED_INT;
            break;
        case OSP_UINT64:
            datatype = DCMF_UNSIGNED_LONG_LONG;
            break;
        case OSP_FLOAT:
            datatype = DCMF_FLOAT;
            bytes = count * sizeof(float);
            status = OSPDI_Malloc(&in_abs, bytes);
            OSPU_ERR_POP(status != OSP_SUCCESS,
                        "OSPDI_Malloc returned error in OSPDI_GlobalAllreduce \n");
            OSPDI_ABS(float, in, in_abs, count);
            in = in_abs;
            break;
        default:
            OSPU_ERR_POP(status != DCMF_SUCCESS, "Unsupported OSP_datatype \n");
            break;
        }
    }
    else
    {
        switch (osp_type)
        {
        case OSP_DOUBLE:
            datatype = DCMF_DOUBLE;
            break;
        case OSP_INT32:
            datatype = DCMF_SIGNED_INT;
            break;
        case OSP_INT64:
            datatype = DCMF_SIGNED_LONG_LONG;
            break;
        case OSP_UINT32:
            datatype = DCMF_UNSIGNED_INT;
            break;
        case OSP_UINT64:
            datatype = DCMF_UNSIGNED_LONG_LONG;
            break;
        case OSP_FLOAT:
            datatype = DCMF_FLOAT;
            break;
        default:
            OSPU_ERR_ABORT(status != DCMF_SUCCESS, "Unsupported OSP_datatype \n");
            break;
        }
    }

    ga_active += 1;
    done_callback.function = OSPDI_Generic_done;
    done_callback.clientdata = (void *) &ga_active;

    status = DCMF_Allreduce(&OSPD_GlobalAllreduce_protocol,
                            &ar_crequest,
                            done_callback,
                            DCMF_SEQUENTIAL_CONSISTENCY,
                            &geometry,
                            (char *) in,
                            (char *) out,
                            count,
                            datatype,
                            reduce_op);

    OSPDI_Conditional_advance(ga_active > 0);

    fn_exit:
    if (in_abs != NULL) 
        OSPDI_Free(in_abs);
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPD_Allreduce_group(OSP_group_t* group,
                        int count,
                        OSP_reduce_op_t osp_op,
                        OSP_datatype_t osp_type,
                        void* in,
                        void* out)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_GlobalAllreduce(count, osp_op, osp_type, in, out);
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_GlobalAllreduce returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_Allreduce_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit: OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPD_NbAllreduce_group(OSP_group_t* group,
                          int count,
                          OSP_reduce_op_t osp_op,
                          OSP_datatype_t osp_type,
                          void* in,
                          void* out,
                          OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        OSPU_ERR_POP(1,
                    "OSPDI_NbAllreduce has not been implemented \n");
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_NbAllreduce_group not implemented for non-world groups!");
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPDI_GlobalBcast_initialize()
{
    int status = OSP_SUCCESS;
    DCMF_GlobalBcast_Configuration_t conf;

    OSPU_FUNC_ENTER();

    conf.protocol = DCMF_DEFAULT_GLOBALBCAST_PROTOCOL;
    status = DCMF_GlobalBcast_register(&OSPD_GlobalBcast_protocol,
                                       &conf);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_GlobalBcast_register returned with error %d \n",
                status);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;
}

int OSPDI_GlobalBcast(int root,
                     int count,
                     void *buffer)
{
    int status = OSP_SUCCESS;
    DCMF_Request_t request;
    DCMF_Callback_t done_callback;
    volatile unsigned gb_active = 0;

    OSPU_FUNC_ENTER();

    gb_active += 1;
    done_callback.function = OSPDI_Generic_done;
    done_callback.clientdata = (void *) &gb_active;

    status = DCMF_GlobalBcast(&OSPD_GlobalBcast_protocol,
                              &request,
                              done_callback,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              root,
                              (char *) buffer,
                              count);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_GlobalBcast returned with error %d \n",
                status);

    OSPDI_Conditional_advance(gb_active > 0);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPD_Bcast_group(OSP_group_t* group,
                    int root,
                    int count,
                    void* buffer)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        status = OSPDI_GlobalBcast(root, count, buffer);
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_GlobalBcast returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_Bcast_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int OSPDI_NbGlobalBcast(int root,
                       int count,
                       void *buffer,
                       OSPD_Handle_t *ospd_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Request_t *ospd_request;
    DCMF_Callback_t done_callback;

    OSPU_FUNC_ENTER();

    ospd_request = OSPDI_Get_request(1);
    OSPU_ERR_POP(status = (ospd_request == NULL),
                "OSPDI_Get_request returned error \n");
    OSPDI_Set_handle(ospd_request, ospd_handle);

    ospd_handle->active++;

    done_callback.function = OSPDI_Request_done;
    done_callback.clientdata = (void *) ospd_request;

    status = DCMF_GlobalBcast(&OSPD_GlobalBcast_protocol,
                              &(ospd_request->request),
                              done_callback,
                              DCMF_SEQUENTIAL_CONSISTENCY,
                              root,
                              (char *) buffer,
                              count);
    OSPU_ERR_POP(status != DCMF_SUCCESS,
                "DCMF_GlobalBcast returned with error %d \n",
                status);

    fn_exit:
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}

int OSPD_NbBcast_group(OSP_group_t* group,
                      int root,
                      int count,
                      void* buffer,
                      OSP_handle_t osp_handle)
{
    int status = OSP_SUCCESS;
    OSPD_Handle_t *ospd_handle;

    OSPU_FUNC_ENTER();

    OSPDI_CRITICAL_ENTER();

    if (group == OSP_GROUP_WORLD || group == NULL)
    {
        ospd_handle = (OSPD_Handle_t *) osp_handle;

        status = OSPDI_NbGlobalBcast(root, count, buffer, ospd_handle);
        OSPU_ERR_ABORT(status != OSP_SUCCESS,
                      "OSPDI_NbGlobalBcast returned with an error");
        goto fn_exit;
    }
    else
    {
        OSPU_ERR_POP(1,
                    "OSPD_Bcast_group not implemented for non-world groups!");
        goto fn_fail;
    }

    fn_exit:
    OSPDI_CRITICAL_EXIT();
    OSPU_FUNC_EXIT();
    return status;

    fn_fail:
    goto fn_exit;

}
