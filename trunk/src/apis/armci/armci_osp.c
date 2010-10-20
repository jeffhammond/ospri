/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "armci.h"
#include "osp.h"
#include "ospd.h"
#include "ospu.h"
#include "assert.h"

/* #define OSP_ARMCI_PROFILING */

#ifdef OSP_ARMCI_PROFILING
int __osp_prof_me = -1;
char* __osp_prof_name[32];
double __osp_prof_t0;
double __osp_prof_t1;
double __osp_prof_dt;

#define AAP_INIT()                                    \
        do {                                              \
            __osp_prof_me = OSP_Process_id(OSP_GROUP_WORLD); \
        } while (0)

#define AAP_START(a)                      \
        do {                                  \
            __osp_prof_name[0] = a;            \
            __osp_prof_t0 = OSP_Time_seconds(); \
        } while (0)

#define AAP_STOP()                                                                      \
        do {                                                                                \
            __osp_prof_t1 = OSP_Time_seconds();                                               \
            __osp_prof_dt = __osp_prof_t1 - __osp_prof_t0;                                     \
            printf("iam %d: %s took %10.4lf s\n",__osp_prof_me,__osp_prof_name,__osp_prof_dt); \
            fflush(stdout);                                                                 \
        } while (0)

#define AAP_ARGS printf

#else
#define AAP_INIT()
#define AAP_START(a)
#define AAP_STOP(a)
#define AAP_ARGS(...)
#endif

/* TODO We really should have ARMCI-to-OSP type/op conversion functions/macros
 *      instead of repeating that code so many times */

int ARMCI_Init_args(int *argc, char ***argv)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_INIT();

    status = OSP_Initialize(OSP_THREAD_SINGLE);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Initialize returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int ARMCI_Init(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_INIT();

    status = OSP_Initialize(OSP_THREAD_SINGLE);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Initialize returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int ARMCI_Finalize(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Finalize();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSPD_Finalize returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

int ARMCI_Malloc(void* ptr[], armci_size_t bytes)
{
    int status = OSP_SUCCESS;
    int my_rank;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    my_rank = OSP_Process_id(OSP_GROUP_WORLD);

    if (bytes == 0)
    {
        ptr[my_rank] = NULL;
    }
    else
    {
        status = OSP_Alloc_segment(&ptr[my_rank], bytes);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Alloc_segment returned an error\n");
    }

    status = OSP_Exchange_segments(OSP_GROUP_WORLD, ptr);
    OSPU_ERR_POP(status != OSP_SUCCESS,
            "OSP_Exchange_segments returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;

}

void* ARMCI_Malloc_local(armci_size_t bytes)
{
    int status = OSP_SUCCESS;
    void *segment_ptr;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Alloc_segment(&segment_ptr, bytes);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Alloc_segement returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return segment_ptr;

    fn_fail: goto fn_exit;
}

int ARMCI_Free(void *ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Release_segments(OSP_GROUP_WORLD, ptr);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Release_segments returned an error\n");

    status = OSP_Free_segment(ptr);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Free_segment returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Free_local(void *ptr)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Free_segment(ptr);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Free_segment returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_INIT_HANDLE(armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Allocate_handle(&osp_handle);
    OSPU_ERR_ABORT(status != OSP_SUCCESS,
            "OSP_Allocate_handle returned an error\n");

    *handle = osp_handle;

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Put(void* src, void* dst, int bytes, int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_ARGS("iam %d: OSP_Put proc = %d, bytes = %d\n",__osp_prof_me,proc,bytes);AAP_START("OSP_Put          ");

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_Put(proc, src, dst, bytes);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Put returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbPut(void* src, void* dst, int bytes, int proc, armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    AAP_ARGS("iam %d: OSP_NbPut proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_NbPutS          ");
    status = OSP_NbPut(proc, src, dst, bytes, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPut returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_PutS(void* src_ptr,
               int src_stride_ar[],
               void* dst_ptr,
               int dst_stride_ar[],
               int count[],
               int stride_levels,
               int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_ARGS("iam %d: OSP_PutS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_PutS          ");

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_PutS(proc,
                     stride_levels,
                     count,
                     src_ptr,
                     src_stride_ar,
                     dst_ptr,
                     dst_stride_ar);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutS returned an error\n");AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbPutS(void* src_ptr,
                 int src_stride_ar[],
                 void* dst_ptr,
                 int dst_stride_ar[],
                 int count[],
                 int stride_levels,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    AAP_ARGS("iam %d: OSP_NbPutS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_NbPutS          ");
    status = OSP_NbPutS(proc,
                       stride_levels,
                       count,
                       src_ptr,
                       src_stride_ar,
                       dst_ptr,
                       dst_stride_ar,
                       osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutS returned an error\n");AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_PutV(armci_giov_t *dsrc_arr, int arr_len, int proc)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    /* ARMCI iov and OSP iov are similar structures but follow 
     * different naming conventions. So we make a copy.*/
    /* TODO Why not use OSPD_Malloc here? OSPD_Malloc is not exposed here, currently*/
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_PutV(proc, osp_iov_ar, arr_len);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutV returned an error\n");

    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbPutV(armci_giov_t *dsrc_arr,
                 int arr_len,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    /* ARMCI iov and OSP iov are similar structures but follow
     * different naming conventions. So we make a copy.*/
    /* TODO Why not use OSPD_Malloc here? OSPD_Malloc is not exposed here, currently*/
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    status = OSP_NbPutV(proc, osp_iov_ar, arr_len, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutV returned an error\n");

    /* it is okay to free this as soon as the function returns?  so we copy it upon
     * entry into OSPD_NbPutV? */
    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Get(void* src, void* dst, int bytes, int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_ARGS("iam %d: OSP_Get proc = %d, bytes = %d\n",__osp_prof_me,proc,bytes);
    AAP_START("OSP_Get         ");

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_Get(proc, src, dst, bytes);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Get returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbGet(void* src, void* dst, int bytes, int proc, armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    AAP_ARGS("iam %d: OSP_NbGet proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_NbGet           ");
    status = OSP_NbGet(proc, src, dst, bytes, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbGet returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_GetS(void* src_ptr,
               int src_stride_ar[],
               void* dst_ptr,
               int dst_stride_ar[],
               int count[],
               int stride_levels,
               int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    AAP_ARGS("iam %d: OSP_GetS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_GetS           ");

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_GetS(proc,
                     stride_levels,
                     count,
                     src_ptr,
                     src_stride_ar,
                     dst_ptr,
                     dst_stride_ar);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_GetS returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbGetS(void* src_ptr,
                 int src_stride_ar[],
                 void* dst_ptr,
                 int dst_stride_ar[],
                 int count[],
                 int stride_levels,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    AAP_ARGS("iam %d: OSP_NbGetS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_NbGetS           ");
    status = OSP_NbGetS(proc,
                       stride_levels,
                       count,
                       src_ptr,
                       src_stride_ar,
                       dst_ptr,
                       dst_stride_ar,
                       osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutS returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_GetV(armci_giov_t *dsrc_arr, int arr_len, int proc)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    /* ARMCI iov and OSP iov are similar structures but follow
     * different naming conventions. So we make a copy.*/

    /* TODO Why not use OSPD_Malloc here? */
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }

    status = OSP_GetV(proc, osp_iov_ar, arr_len);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_GetV returned an error\n");

    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbGetV(armci_giov_t *dsrc_arr,
                 int arr_len,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    /* ARMCI iov and OSP iov are similar structures but follow
     * different naming conventions. So we make a copy.*/

    /* TODO Why not use OSPD_Malloc here? */
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    status = OSP_NbGetV(proc, osp_iov_ar, arr_len, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbGetV returned an error\n");

    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Acc(int datatype,
              void *scale,
              void* src,
              void* dst,
              int bytes,
              int proc)
{
    int status = OSP_SUCCESS;
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    AAP_ARGS("iam %d: OSP_PutAcc proc = %d, bytes = %d\n",__osp_prof_me,proc,bytes);
    AAP_START("OSP_PutAcc             ");

    /* accumulate flushes puts before and holds the lock throughout
    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }
    */

    status = OSP_PutAcc(proc, src, dst, bytes, osp_type, scale);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutAcc returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbAcc(int datatype,
                void *scale,
                void* src,
                void* dst,
                int bytes,
                int proc,
                armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_datatype_t osp_type;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    AAP_ARGS("iam %d: OSP_NbPutAcc proc = %d, bytes = %d\n",__osp_prof_me,proc,bytes);
    AAP_START("OSP_NbPutAcc             ");
    status = OSP_NbPutAcc(proc, src, dst, bytes, osp_type, scale, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutAcc returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_AccS(int datatype,
               void *scale,
               void* src_ptr,
               int src_stride_ar[],
               void* dst_ptr,
               int dst_stride_ar[],
               int count[],
               int stride_levels,
               int proc)
{
    int status = OSP_SUCCESS;
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
        case ARMCI_ACC_LNG:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype %d \n", datatype);
    }

    AAP_ARGS("iam %d: OSP_PutAccS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_PutAccS             ");

    /* accumulate flushes puts before and holds the lock throughout
    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }
    */

    status = OSP_PutAccS(proc,
                        stride_levels,
                        count,
                        src_ptr,
                        src_stride_ar,
                        dst_ptr,
                        dst_stride_ar,
                        osp_type,
                        scale);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutAccS returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbAccS(int datatype,
                 void *scale,
                 void* src_ptr,
                 int src_stride_ar[],
                 void* dst_ptr,
                 int dst_stride_ar[],
                 int count[],
                 int stride_levels,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_datatype_t osp_type;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    AAP_ARGS("iam %d: OSP_NbPutAccS proc = %d, levels = %d, count[0] = %d, count[1] = %d\n",__osp_prof_me,proc,stride_levels,count[0],count[stride_levels-1]);
    AAP_START("OSP_NbPutAccS             ");
    status = OSP_NbPutAccS(proc,
                          stride_levels,
                          count,
                          src_ptr,
                          src_stride_ar,
                          dst_ptr,
                          dst_stride_ar,
                          osp_type,
                          scale,
                          osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutAccS returned an error\n");
    AAP_STOP();

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_AccV(int datatype,
               void *scale,
               armci_giov_t *dsrc_arr,
               int arr_len,
               int proc)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;
    OSP_reduce_op_t osp_op; /* only used by PutModV */
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_RA:
            osp_type = OSP_INT32;
            osp_op = OSP_BXOR;
            status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t) * arr_len);
            OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");
            memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);
            status = OSP_PutModV(proc, osp_iov_ar, arr_len, osp_op, osp_type);
            OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutModV returned an error\n");
            free(osp_iov_ar);
            goto fn_exit;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    /* ARMCI iov and OSP iov are similar structures but follow
     * different naming conventions. So we make a copy.*/
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    /* accumulate flushes puts before and holds the lock throughout
    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }
    */

    status = OSP_PutAccV(proc, osp_iov_ar, arr_len, osp_type, scale);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_PutAccV returned an error\n");

    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_NbAccV(int datatype,
                 void *scale,
                 armci_giov_t *dsrc_arr,
                 int arr_len,
                 int proc,
                 armci_hdl_t* handle)
{
    int status = OSP_SUCCESS;
    OSP_iov_t *osp_iov_ar;
    OSP_datatype_t osp_type;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    /* ARMCI iov and OSP iov are similar structures but follow
     * different naming conventions. So we make a copy.*/
    status = posix_memalign((void **) &osp_iov_ar, 16, sizeof(OSP_iov_t)
            * arr_len);
    OSPU_ERR_POP(status != 0, "posix_memalign returned an error\n");

    memcpy((void *) osp_iov_ar, (void *) dsrc_arr, sizeof(OSP_iov_t) * arr_len);

    status = OSP_NbPutAccV(proc, osp_iov_ar, arr_len, osp_type, scale, osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_NbPutAccV returned an error\n");

    free(osp_iov_ar);

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Rmw(int op, void *ploc, void *prem, int value, int proc)
{

    int status = OSP_SUCCESS;
    OSP_atomic_op_t osp_op;

    OSPU_FUNC_ENTER();

#   ifdef HAVE_ERROR_CHECKING
#   endif

    if (op == ARMCI_FETCH_AND_ADD || op == ARMCI_FETCH_AND_ADD_LONG)
    {
        osp_op = OSP_FETCH_AND_ADD;
    }
    else if (op == ARMCI_SWAP || op == ARMCI_SWAP_LONG)
    {
        osp_op = ARMCI_SWAP;
    }
    else
    {
        OSPU_ERR_POP(status != OSP_ERROR, "Unsupported rmw operation : %d \n", op);
    }

    /* accumulate flushes puts before and holds the lock throughout
    if ( 1==ospu_settings.armci_strict_ordering )
    {
        status = OSP_Flush(proc);
        OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Flush returned an error\n");
    }
    */

    /*Assuming int and long as 32bit signed integers*/
    status = OSP_Rmw(proc, &value, ploc, prem, sizeof(int), osp_op, OSP_INT32);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Rmw returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Wait(armci_hdl_t* handle)
{

    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    status = OSP_Wait_handle(osp_handle);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Wait_handle returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Test(armci_hdl_t* handle)
{

    int status = OSP_SUCCESS;
    OSP_handle_t osp_handle;
    OSP_bool_t complete;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    osp_handle = (OSP_handle_t) *handle;

    status = OSP_Test_handle(osp_handle, &complete);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Test_handle returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return !complete;

    fn_fail: goto fn_exit;
}

int ARMCI_WaitAll(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Wait_handle_all();
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Wait_handle_all returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_Fence(int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Flush(proc);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Flush returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_AllFence(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Flush_group(OSP_GROUP_WORLD);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Flush_group returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Barrier(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    status = OSP_Sync_group(OSP_GROUP_WORLD);
    OSPU_ERR_POP(status != OSP_SUCCESS, "OSP_Sync_group returned an error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int armci_msg_nproc(void)
{
    int nproc;

    OSPU_FUNC_ENTER();

    /* FIXME: The profiling interface needs to go here */

    /* FIXME: Locking functionality needs to go here */

#   ifdef HAVE_ERROR_CHECKING
#   endif

    nproc = OSP_Process_total(OSP_GROUP_WORLD);

    fn_exit: OSPU_FUNC_EXIT();
    return nproc;

    fn_fail: goto fn_exit;
}

int armci_msg_me(void)
{
    int me;

    OSPU_FUNC_ENTER();

    me = OSP_Process_id(OSP_GROUP_WORLD);

    fn_exit: OSPU_FUNC_EXIT();
    return me;

    fn_fail: goto fn_exit;
}

int armci_domain_id(armci_domain_t domain, int glob_proc_id)
{
    int domain_id;

    OSPU_FUNC_ENTER();

    domain_id = 0;

    fn_exit: OSPU_FUNC_EXIT();
    return domain_id;

    fn_fail: goto fn_exit;
}

/**********************************************
 Some Dummy ARMCI Function implemenations
 but have to be implemented soon
 ***********************************************/

int ARMCI_Create_mutexes(int num)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_Lock(int mutex, int proc)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_Unlock(int mutex, int proc)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Destroy_mutexes(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_Copy(void *src, void *dst, int n)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

/*************************************************
 Some More Dummy ARMCI Function implementations
 which are less important
 **************************************************/

int ARMCI_Absolute_id(ARMCI_Group *group, int group_rank)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_Cleanup(void)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_Error(char *message, int code)
{
    OSPU_FUNC_ENTER();

    OSP_Abort(code, message);

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Uses_shm(void)
{
    OSPU_FUNC_ENTER();

    fn_exit: OSPU_FUNC_EXIT();
    return 0;

    fn_fail: goto fn_exit;
}

void ARMCI_Set_shm_limit(unsigned long shmemlimit)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Uses_shm_grp(ARMCI_Group *group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void ARMCI_Group_get_world(ARMCI_Group *group_out)
{
    OSPU_FUNC_ENTER();

    *group_out = OSP_GROUP_WORLD;

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_Group_set_default(ARMCI_Group *group)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_Group_create(int n, int *pid_list, ARMCI_Group *group_out)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP \n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void ARMCI_Group_free(ARMCI_Group *group)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_Free_group(void *ptr, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Malloc_group(void *ptr_arr[], armci_size_t bytes, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void armci_msg_gop_scope(int scope, void *x, int n, char* op, int type)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    if (scope != SCOPE_ALL)
    {
        OSPU_ERR_ABORT(OSP_ERROR, "Only SCOPE_ALL is supported in armci_msg_gop_scope");
    }

    switch (type)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
            osp_type = OSP_DOUBLE;
            break;
        default:
            OSPU_ERR_ABORT(OSP_ERROR, "Invalid datatype received in armci_msg_group_gop_scope");
            break;
    }

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_ABORT(OSP_ERROR, "Invalid op received in armci_msg_group_gop_scope");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                osp_type,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_snd(int tag, void* buffer, int len, int to)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_rcv(int tag, void* buffer, int buflen, int *msglen, int from)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int armci_msg_rcvany(int tag, void* buffer, int buflen, int *msglen)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

void armci_write_strided(void *ptr,
                         int stride_levels,
                         int stride_arr[],
                         int count[],
                         char *buf)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_read_strided(void *ptr,
                        int stride_levels,
                        int stride_arr[],
                        int count[],
                        char *buf)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

int ARMCI_PutS_flag_dir(void *src_ptr,
                        int src_stride_arr[],
                        void* dst_ptr,
                        int dst_stride_arr[],
                        int count[],
                        int stride_levels,
                        int *flag,
                        int val,
                        int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_PutS_flag(void *src_ptr,
                    int src_stride_arr[],
                    void* dst_ptr,
                    int dst_stride_arr[],
                    int count[],
                    int stride_levels,
                    int *flag,
                    int val,
                    int proc)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return status;

    fn_fail: goto fn_exit;
}

int ARMCI_Same_node(int proc)
{
    int val;

    OSPU_FUNC_ENTER();

    /* each process is its own node */
    val = (proc == OSP_Process_id(OSP_GROUP_WORLD) ? 1 : 0);

    fn_exit: OSPU_FUNC_EXIT();
    return val;

    fn_fail: goto fn_exit;
}

int armci_domain_same_id(armci_domain_t domain, int proc)
{
    int val;

    OSPU_FUNC_ENTER();

    /* this function always returns false */
    val = 0;

    fn_exit: OSPU_FUNC_EXIT();
    return val;

    fn_fail: goto fn_exit;
}

int armci_domain_my_id(armci_domain_t domain)
{
    int val;

    OSPU_FUNC_ENTER();

    /* this function always returns false */
    val = 0;

    fn_exit: OSPU_FUNC_EXIT();
    return val;

    fn_fail: goto fn_exit;
}

int armci_domain_count(armci_domain_t domain)
{
    int val;

    OSPU_FUNC_ENTER();

    /* this function always returns one */
    val = 1;

    fn_exit: OSPU_FUNC_EXIT();
    return val;

    fn_fail: goto fn_exit;
}

int armci_domain_nprocs(armci_domain_t domain, int id)
{
    int nproc;

    OSPU_FUNC_ENTER();

    nproc = OSP_Process_total(OSP_GROUP_WORLD);

    fn_exit: OSPU_FUNC_EXIT();
    return nproc;

    fn_fail: goto fn_exit;
}

int armci_domain_glob_proc_id(armci_domain_t domain, int id, int loc_proc_id)
{
    int val;

    OSPU_FUNC_ENTER();

    /* this function always returns zero */
    val = 0;

    fn_exit: OSPU_FUNC_EXIT();
    return val;

    fn_fail: goto fn_exit;
}

void armci_msg_llgop(long long *x, int n, char* op)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT64,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_bcast(void* buffer, int len, int root)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    status = OSP_Bcast_group(OSP_GROUP_WORLD, root, len, buffer);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Bcast_group returned error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_barrier(void)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    status = OSP_Barrier_group(OSP_GROUP_WORLD);

    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Barrier_group returned error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_dgop(double *x, int n, char* op)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_DOUBLE,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_fgop(float *x, int n, char* op)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_FLOAT,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_igop(int *x, int n, char* op)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT32,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_lgop(long *x, int n, char* op)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT32,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_reduce(void *x, int n, char* op, int datatype)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    switch (datatype)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
        case ARMCI_ACC_INT:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
        case ARMCI_ACC_FLT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
        case ARMCI_ACC_DBL:
            osp_type = OSP_DOUBLE;
            break;
        case ARMCI_ACC_CPL:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_CPL datatype not supported\n");
        case ARMCI_ACC_DCP:
            OSPU_ERR_ABORT(status != OSP_ERROR, "ARMCI_ACC_DCP datatype not supported\n");
        default:
            OSPU_ERR_ABORT(status != OSP_ERROR, "invalid datatype\n");
    }

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                osp_type,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_bintree(int scope, int* Root, int *Up, int *Left, int *Right)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_exchange_address(void *ptr_ar[], int n)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_group_igop(int *x, int n, char* op, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT32,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_group_lgop(long *x, int n, char* op, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT32,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_group_llgop(long long *x, int n, char* op, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_INT64,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_group_fgop(float *x, int n, char* op, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_FLOAT,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_group_dgop(double *x, int n, char* op, ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;

    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else  OSPU_ERR_POP(OSP_ERROR, "Invalid op received\n");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                OSP_DOUBLE,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_msg_group_bcast_scope(int scope,
                                 void *buf,
                                 int len,
                                 int root,
                                 ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSPU_FUNC_ENTER();

    if (scope != SCOPE_ALL)
    {
        OSPU_ERR_ABORT(OSP_ERROR, "Only SCOPE_ALL is supported \n");
    }

    /*We need a check if it is a world group, we assume it for now*/

    status = OSP_Bcast_group(OSP_GROUP_WORLD, root, len, buf);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Bcast_group returned error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_group_barrier(ARMCI_Group *group)
{
    OSPU_FUNC_ENTER();

    /*We need a check if it is a world group, we assume it for now*/

    OSP_Barrier_group(OSP_GROUP_WORLD);

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_group_gop_scope(int scope,
                               void *x,
                               int n,
                               char* op,
                               int type,
                               ARMCI_Group *group)
{
    int status = OSP_SUCCESS;
    OSP_reduce_op_t osp_op;
    OSP_datatype_t osp_type;

    OSPU_FUNC_ENTER();

    if (scope != SCOPE_ALL)
    {
        OSPU_ERR_ABORT(OSP_ERROR, "Only SCOPE_ALL is supported in armci_msg_gop_scope");
    }

    /*We need a check if it is a world group, we assume it for now*/

    switch (type)
    {
        case ARMCI_INT:
        case ARMCI_LONG:
            osp_type = OSP_INT32;
            break;
        case ARMCI_LONG_LONG:
            osp_type = OSP_INT64;
            break;
        case ARMCI_FLOAT:
            osp_type = OSP_FLOAT;
            break;
        case ARMCI_DOUBLE:
            osp_type = OSP_DOUBLE;
            break;
        default:
            OSPU_ERR_ABORT(OSP_ERROR, "Invalid datatype received in armci_msg_group_gop_scope");
            break;
    }

    if (strncmp(op, "+", 1) == 0) osp_op = OSP_SUM;
    else if (strncmp(op, "*", 1) == 0) osp_op = OSP_PROD;
    else if (strncmp(op, "max", 3) == 0) osp_op = OSP_MAX;
    else if (strncmp(op, "min", 3) == 0) osp_op = OSP_MIN;
    else if (strncmp(op, "absmax", 6) == 0) osp_op = OSP_MAXABS;
    else if (strncmp(op, "absmin", 6) == 0) osp_op = OSP_MINABS;
    else if (strncmp(op, "or", 2) == 0) osp_op = OSP_OR;
    else OSPU_ERR_ABORT(OSP_ERROR, "Invalid op received in armci_msg_group_gop_scope");

    status = OSP_Allreduce_group(OSP_GROUP_WORLD,
                                n,
                                osp_op,
                                osp_type,
                                x,
                                x);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Allreduce_group returned error\n");

    fn_exit:
    OSPU_FUNC_EXIT();
    return;

    fn_fail:
    goto fn_exit;
}

void armci_grp_clus_brdcst(void *buf,
                           int len,
                           int grp_master,
                           int grp_clus_nproc,
                           ARMCI_Group *mastergroup)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_exchange_address_grp(void *ptr_arr[], int n, ARMCI_Group *group)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_bcast_scope(int scope, void* buffer, int len, int root)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_brdcst(void* buffer, int len, int root)
{
    int status = OSP_SUCCESS;

    OSPU_FUNC_ENTER();

    status = OSP_Bcast_group(OSP_GROUP_WORLD, root, len, buffer);
    OSPU_ERR_ABORT(status != OSP_SUCCESS, "OSP_Bcast_group returned error\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}

void armci_msg_sel_scope(int scope,
                         void *x,
                         int n,
                         char* op,
                         int type,
                         int contribute)
{
    OSPU_FUNC_ENTER();

    OSPU_ERR_ABORT(OSP_ERROR, "This function is not supported in ARMCI-OSP\n");

    fn_exit: OSPU_FUNC_EXIT();
    return;

    fn_fail: goto fn_exit;
}
