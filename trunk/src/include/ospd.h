/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "osp.h"
#include "ospu.h"

#if !defined OSPD_H_INCLUDED
#define OSPD_H_INCLUDED

/* ********************************************************************* */
/*                                                                       */
/*               OSP data structures                                      */
/*                                                                       */
/* ********************************************************************* */



/* ********************************************************************* */
/*                                                                       */
/*               OSP device-level functions                               */
/*                                                                       */
/* ********************************************************************* */

/**
 * \brief Device level implementation of OSP_Initialize.
 *
 * \param[out] rc               The error code from initializing OSP
 * \param[in]  OSP_thread_level  The type of thread support for OSP
 *
 * \ingroup MANAGEMENT
 */
int OSPD_Initialize(int OSP_thread_level);

/**
 * \brief Device level implementation of OSP_Finalize.
 *
 * \param[out] rc  The error code from terminating OSP.  
 *
 * \ingroup MANAGEMENT
 */
int OSPD_Finalize(void);

#if 0

/**
 * \brief Device level implementation of OSP_Abort.
 *
 * \param[in]  error_code    The error code to be returned to the submitting environment.
 * \param[in]  error_message Text string to print to stderr upon termination.
 *
 * \ingroup MANAGEMENT
 */

void OSPD_Abort(int error_code, char error_message[]);

/**
 * \brief Device level implementation of OSPD_Alloc_segment.
 *
 * A local operation to allocate memory to be used in context of OSP copy operations.
 *
 * \note Memory allocated with this function will be properly aligned for the architecture.
 *
 * \warning Memory allocated with this function must be freed by OSP_Free_segment.
 *
 * \param[out] rc            The error code.
 * \param[out] ptr           Pointer to memory.
 * \param[in]  bytes         The amount of memory requested.
 *
 * \ingroup MEMORY
 */
int OSPD_Alloc_segment(void** pointer, int bytes);

/**
 * \brief Device level implementation of OSPD_Free_segment.
 *
 * A local operation to free memory allocated by OSP_Alloc_segment.
 *
 * \warning It is erroneous to attempt to free memory not allocated by OSP_Alloc_segment.
 *
 * \param[out] rc            The error code.
 * \param[in] ptr           Pointer to memory.
 *
 * \ingroup MEMORY
 */

int OSPD_Free_segment(void* pointer);

/**
 * \brief Device level implementation of OSP_Exchange_segments.
 *
 *  A collective operation to allocate memory to be used in context of OSP copy operations.
 *
 * \param[out] rc         The error code.
 * \param[in]  group      Group of processes within which the pointer list is exchanged.
 * \param[in]  ptr        Pointer array. Each one points to memory allocated at one process, 
 *                        in order of ranks.
 * \param[in]  bytes      The size of memory allocated at each process.
 *
 * \ingroup MEMORY 
 */
int OSPD_Exchange_segments(OSP_group_t* group, void **ptr);

/**
 * \brief Device level implementation of OSP_Release_segments.
 *
 * A collective operation to invalidate and de-register memory segments
 * associated with an OSPD_Exchange_segments call. 
 *
 * \param[out] rc          The error code.
 * \param[in]  group       Group of processes within which the pointer list was exchanged.
 * \param[in]  ptr         Pointer to the allocated memory.
 *
 * \ingroup MEMORY 
 */
int OSPD_Release_segments(OSP_group_t* group, void *ptr);

/**
 * \brief Device level implementation of OSP_Allocate_handle.
 *
 * Allocates a non-blocking handle.
 *
 * \param[in] handle      Non-blocking handle upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Allocate_handle(OSP_handle_t *handle);


/**
 * \brief Device level implementation of OSP_Release_handle.
 * 
 * Releases a non-blocking handle.
 * 
 * \param[in] handle      Non-blocking handle upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Release_handle(OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_Wait_handle_all.
 *
 * Waits for operations on all handle to complete.
 *
 * \param[in] handle      Non-blocking handle upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Wait_handle_all(void);

/**
 * \brief Device level implementation of OSP_Wait_handle.
 *
 * Waits for operations on a handle to complete.
 *
 * \param[in] handle      Non-blocking handle upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Wait_handle(OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_Wait_handle_list.
 *
 * Waits for operations on a list of handles to complete.
 *
 * \param[in] count          Number of handles
 * \param[in] osp_handle      Non-blocking handles upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Wait_handle_list(int count, OSP_handle_t *osp_handle);

/**
 * \brief Device level implementation of OSP_Test_handle.
 *
 * Test for completion of operations on a handle.
 *
 * \param[in] handle      Non-blocking handle upon which to be waited.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */

int OSPD_Test_handle(OSP_handle_t handle, OSP_bool_t* completed);

/**
 * \brief Device level implementation of OSP_Test_handle_list.
 *
 * Test for completion of operations on a list of handles.
 *
 * \param[in] count          Number of handles
 * \param[in] osp_handle      Non-blocking handles upon which to be tested.
 *
 * \see OSP_handle_t, OSP_Wait_handle_list, OSP_Test_handle
 *
 * \ingroup MEMORY
 */
int OSPD_Test_handle_list(int count,
                         OSP_handle_t *osp_handle,
                         OSP_bool_t* *completed);

/**
 * \brief Device level implementation of OSP_Barrier_group.
 *
 * On return, this call ensures that all processes within the entire group
 * have reached this point in the program.
 *
 * \param[in] group          Group of processes to synchronize.
 *
 * \ingroup  SYNCHRONIZATION
 */
int OSPD_Barrier_group(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_NbBarrier_group.
 *
 * \param[in] group          Group of processes to synchronize.
 *
 * \ingroup  SYNCHRONIZATION
 */
int OSP_NbBarrier_group(OSP_group_t* group, OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_Sync_group.
 *
 * On return, this call ensures that all processes within the entire group
 * have reached this point in the program and that all messages have completed remotely.
 *
 * \param[in] group          Group of processes to synchronize.
 *
 * \ingroup  SYNCHRONIZATION
 */
int OSPD_Sync_group(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_NbSync_group.
 *
 * \param[in] group          Group of processes to synchronize.
 *
 * \ingroup  SYNCHRONIZATION
 */
int OSPD_NbSync_group(OSP_group_t* group, OSP_handle_t handle);

#endif

/**
 * \brief Device level implementation of OSP_Put.
 *
 * Blocking copy of contiguous data from local memory to remote memory.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (local) source memory.
 * \param[in]  target_ptr    Starting address in the (remote) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes. 
 *
 * \ingroup  COPY OPERATIONS
 */
int OSPD_Put(int target, void* src, void* dst, int bytes);

#if 0

/**
 * \brief Device level implementation of OSP_NbPut.
 *
 * Non-Blocking copy of contiguous data from local memory to remote memory.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (local) source memory.
 * \param[in]  target_ptr    Starting address in the (remote) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes.
 * \param[in]  handle        Opaque OSP handle for request
 *
 * \ingroup  COPY OPERATIONS
 */
int OSPD_NbPut(int target, void* src, void* dst, int bytes, OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_PutS.
 *
 * Blocking copy of non-contiguous (strided) data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level   The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (local) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (remote) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_PutS(int target,
             int stride_level,
             int block_sizes[],
             void* source_ptr,
             int src_stride_ar[],
             void* target_ptr,
             int trg_stride_ar[]);

/**
 * \brief Device level implementation of OSP_NbPutS.
 * 
 * Non-Blocking copy of non-contiguous (strided) data from local memory to remote memory.
 * 
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level   The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (local) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (remote) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 * \param[in]  handle        Opaque OSP handle for request
 *
 * \ingroup COPY OPERATIONS
 */

int OSPD_NbPutS(int target,
               int stride_level,
               int block_sizes[],
               void* source_ptr,
               int src_stride_ar[],
               void* target_ptr,
               int trg_stride_ar[],
               OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_PutV.
 * 
 *  Blocking copy of non-contiguous data from local memory to remote memory.
 * 
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * 
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 * 
 * \ingroup DATA_TRANSFER
 */

int OSPD_PutV(int target, 
             OSP_iov_t *iov_ar,
             int ar_len);

/**
 * \brief Device level implementation of OSP_NbPutV.
 * 
 *  Non-Blocking copy of non-contiguous data from local memory to remote memory.
 * 
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * \param[in]  osp_handle       OSP Opaque handle
 * 
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 * 
 * \ingroup DATA_TRANSFER
 */

int OSPD_NbPutV(int target, 
               OSP_iov_t *iov_ar,
               int ar_len,
               OSP_handle_t osp_handle);

#endif

/**
 * \brief Device level implementation of OSP_Get.
 *
 * Blocking copy of contiguous data from remote memory to local memory.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (remote) source memory.
 * \param[in]  target_ptr    Starting address in the (local) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes.
 *
 * \ingroup  COPY OPERATIONS
 */
int OSPD_Get(int target, void* src, void* dst, int bytes);

#if 0

/**
 * \brief Device level implementation of OSP_Get.
 *
 * Non-Blocking copy of contiguous data from remote memory to local memory.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (remote) source memory.
 * \param[in]  target_ptr    Starting address in the (local) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes.
 * \param[in]  handle        Opaque OSP handle for request
 *
 * \ingroup  COPY OPERATIONS
 */
int OSPD_NbGet(int target, void* src, void* dst, int bytes, OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_GetS.
 *
 * Blocking copy of non-contiguous (strided) data from remote memory to local memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level   The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (remote) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (local) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_GetS(int target,
             int stride_level,
             int block_sizes[],
             void* source_ptr,
             int src_stride_ar[],
             void* target_ptr,
             int trg_stride_ar[]);

/**
 * \brief Device level implementation of OSP_NbGetS.
 *
 * Non-Blocking copy of non-contiguous (strided) data from remote memory to local memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level    The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (remote) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (local) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 * \param[in]  handle          Opaque OSP handle for request
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_NbGetS(int target,
               int stride_level,
               int block_sizes[],
               void* source_ptr,
               int src_stride_ar[],
               void* target_ptr,
               int trg_stride_ar[],
               OSP_handle_t handle);

/**
 * \brief  Device level implementation of OSP_GetV.
 *
 * Blocking copy of non-contiguous data from remote memory to local memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * 
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 * 
 * \ingroup DATA_TRANSFER
 */

int OSPD_GetV(int target,
             OSP_iov_t *iov_ar,
             int ar_len);

/**
 * \brief Device level implementation of OSP_NbGetV
 *
 * Non-Blocking copy of non-contiguous data from remote memory to local memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * \param[in]  handle          OSP Opaque handle
 * 
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 * 
 * \ingroup DATA_TRANSFER
 */

int OSPD_NbGetV(int target,
               OSP_iov_t *iov_ar,
               int ar_len,
               OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_PutAcc
 *
 * Blocking accumulate of contiguous data from local memory onto remote memory.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (local) source memory.
 * \param[in]  target_ptr    Starting address in the (remote) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes.
 * \param[in]  osp_type       Amount of data to transfer, in bytes.
 * \param[in]  scaling       Factor for scaling source
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_PutAcc(int target,
               void* source_ptr,
               void* target_ptr,
               int bytes,
               OSP_datatype_t osp_type,
               void* scaling);

/**
 * \brief Device level implementation of OSP_NbPutAcc
 * 
 * Non-Blocking accumulate of contiguous data from local memory onto remote memory.
 * 
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the remote process.
 * \param[in]  source_ptr    Starting address in the (local) source memory.
 * \param[in]  target_ptr    Starting address in the (remote) target memory.
 * \param[in]  bytes         Amount of data to transfer, in bytes.
 * \param[in]  osp_type       Amount of data to transfer, in bytes.
 * \param[in]  scaling       Factor for scaling source
 * \param[in]  handle        Opaque OSP handle
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_NbPutAcc(int target,
                 void* source_ptr,
                 void* target_ptr,
                 int bytes,
                 OSP_datatype_t osp_type,
                 void* scaling,
                 OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_PutAccS 
 *
 * Blocking accumulate of non-contiguous (strided) data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level   The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (local) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (remote) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 * \param[in]  osp_type         Amount of data to transfer, in bytes.
 * \param[in]  scaling         Factor for scaling source
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_PutAccS(int target,
                int stride_level,
                int block_sizes[],
                void* source_ptr,
                int *src_stride_ar,
                void* target_ptr,
                int *trg_stride_ar,
                OSP_datatype_t osp_type,
                void* scaling);

/**
 * \brief Device level implementation of OSP_NbPutAccS
 *
 * Non-Blocking accumulate of non-contiguous (strided) data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  stride_level   The number of levels of stride.
 * \param[in]  block_sizes     Block size in each dimension, in bytes.
 * \param[in]  source_ptr      Starting address in the (local) source memory.
 * \param[in]  src_stride_ar   Array of stride distances at source, in bytes.
 * \param[in]  target_ptr      Starting address in the (remote) target memory.
 * \param[in]  trg_stride_ar   Array of stride distances at target, in bytes.
 * \param[in]  osp_type         Amount of data to transfer, in bytes.
 * \param[in]  scaling         Factor for scaling source
 * \param[in]  handle          Opaque OSP handle
 *
 * \ingroup COPY OPERATIONS
 */
int OSPD_NbPutAccS(int target,
                  int stride_level,
                  int block_sizes[],
                  void* source_ptr,
                  int *src_stride_ar,
                  void* target_ptr,
                  int *trg_stride_ar,
                  OSP_datatype_t osp_type,
                  void* scaling,
                  OSP_handle_t handle);

/**
 * \brief Device level implementation of OSP_PutAccV
 * 
 * Blocking accumulate of non-contiguous data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * \param[in]  osp_type         Type of data and scaling factor
 * \param[in]  *scaling        Scaling factor in the accumulate operation.
 *
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 *
 * \ingroup DATA_TRANSFER
 */

int OSPD_PutAccV(int target,
                OSP_iov_t *iov_ar,
                int ar_len,
                OSP_datatype_t osp_type,
                void* scaling);

/**
 * \brief Device level implementation of OSP_PutAccV
 * 
 * Blocking accumulate of non-contiguous data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * \param[in]  osp_type         Type of data and scaling factor
 * \param[in]  *scaling        Scaling factor in the accumulate operation.
 * \param[in]  osp_handle       OSP opaque handle
 *
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 *
 * \ingroup DATA_TRANSFER
 */

int OSPD_NbPutAccV(int target,
                  OSP_iov_t *iov_ar,
                  int ar_len,
                  OSP_datatype_t osp_type,
                  void* scaling,
                  OSP_handle_t osp_handle);

/**
 * \brief Device level implementation of OSP_PutModV
 *
 * \brief Blocking remote modify of non-contiguous data from local memory to remote memory.
 *
 * \param[out] rc              The error code.
 * \param[in]  target          Rank of the remote process.
 * \param[in]  iov_ar          Array of io vectors. Each vector represents a set of
 *                             chunks of same size.
 * \param[in]  ar_len          Number of elements in the array.
 * \param[in]  osp_op           Reduce operation
 * \param[in]  osp_type         Type of data and scaling factor
 *
 * \see OSP_NbPut, OSP_NbPutV, OSP_NbMultiPut, OSP_NbMultiPutS, OSP_NbMultiPutV
 *
 * \ingroup DATA_TRANSFER
 */

int OSPD_PutModV(int target,
                OSP_iov_t *iov_ar,
                int ar_len,
                OSP_reduce_op_t osp_op,
                OSP_datatype_t osp_type);

/**
 * \brief Collective operation to allocate a counter.
 *
 * \param[out] rc            The error code.
 * \param[in]  counter       OSP shared counter.
 * 
 * \ingroup Atomics
 */
int OSPD_Create_counter(OSP_group_t* group, 
                       OSP_counter_t *counter);

/**
 * \brief Collective operation to deallocate and deregister a counter.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the counter is shared.
 * \param[inout]  counter       OSP shared counter.
 *
 * \see osp_counter_t, OSP_Create_counter, OSP_Incr_counter, OSP_NbIncr_counter
 *
 * \ingroup Atomics
 */

int OSPD_Destroy_counter(OSP_group_t* group,
                       OSP_counter_t *counter); 


/**
 * \brief Atomically updates a shared counter and returns the current value.
 *
 * \param[out] rc            The error code.
 * \param[in]  counter       OSP shared counter.
 * \param[in]  increment     The value to add to the counter.
 * \param[in]  original      The remote value of the counter prior to the increment.
 *
 * \see osp_counter_t, OSP_Create_counter, OSP_Destroy_counter, OSP_NbIncr_counter
 *
 * \ingroup Atomics
 */

int OSPD_Incr_counter(OSP_counter_t counter,
                     long increment,
                     long* original);

/**
 * \brief Atomically updates a shared counter and returns the current value.
 *
 * \param[out] rc            The error code.
 * \param[in]  target        Rank of the target process.
 * \param[in]  source_ptr    Pointer of variable at source process.
 * \param[in]  target_ptr    Pointer of variable at target process.
 * \param[in]  op            Operation to be performed.
 * \param[in]  value         Local buffer containing the value and which will contain
 *                           the current value after the operation.
 *
 * \ingroup Atomics
 */
int OSPD_Rmw(int target,
           void* source_ptr_in,
           void* source_ptr_out,
           void* target_ptr,
           int bytes,
           OSP_atomic_op_t op,
           OSP_datatype_t osp_type);


/**
 * \brief Device level implementation of OSP_Create_mutexes
 *
 * Collective operation to allocate and register a list of mutexes.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the mutexes are shared.
 * \param[in]     count         Number of mutexes to be created.
 * \param[int]    count_ar      An arrays storing the number of mutexes on each process
 *
 * \ingroup Atomics
 */
int OSPD_Create_mutexes(OSP_group_t* group, 
                       int mutex_count, 
                       int *mutex_count_ar);

/**
 * \brief Device level implementation of OSP_Destroy_mutexes
 *
 * Collective operation to unregister and deallocate a list of mutexes.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the mutexes are shared.
 *
 * \ingroup Atomics
 */
int OSPD_Destroy_mutexes(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_Lock_mutex
 * 
 * Operation to lock a mutex. Blocks until lock has been acquired.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the mutexes are shared.
 * \param[in]     mutex         OSP mutex.
 * \param[in]     proc          Process on which you want to lock mutex on
 *
 * \ingroup Atomics
 */
int OSPD_Lock_mutex(OSP_group_t* group, 
                   int mutex, 
                   int proc);

/**
 * \brief Device level implementation of OSP_Trylock_mutex
 * 
 * Operation to trylock a mutex.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the mutexes are shared.
 * \param[in]     mutex         OSP mutex.
 * \param[in]     proc          Process on which you want to lock mutex on
 * \param[out]    acquired      returns 1 if was acquired
 *
 * \ingroup Atomics
 */

int OSPD_Trylock_mutex(OSP_group_t* group, 
                      int mutex, 
                      int proc, 
                      OSP_bool_t *acquired);

/**
 * \brief Device level implementation of OSP_Unlock_mutex
 *  
 * Operation to unlock a mutex.  This call blocks until the mutex has been unlocked.
 *
 * \param[out]    rc            The error code.
 * \param[in]     group         OSP group over which the mutexes are shared.
 * \param[in]     mutex         OSP mutex.
 * \param[in]     proc          Process on which you want to lock mutex on
 *
 * \ingroup Atomics
 */

int OSPD_Unlock_mutex(OSP_group_t* group, 
                     int mutex, 
                     int proc);

/**
 * \brief Device level implementation of OSP_Flush 
 * 
 *  On return, this call ensure that all blocking put or accumulate operations
 *  issued to a particular process are complete remotely.
 *
 * \param[in]  proc          Rank of the remote process.
 *
 * \ingroup COMPLETION
 */
int OSPD_Flush(int proc);

/**
 * \brief Device level implementation of OSP_Flush_group
 *
 *  On return, this call ensure that all blocking put or accumulate operations
 *  issued to the group of processes are complete remotely.
 *
 * \param[in]  group          Group of the remote processs.
 *
 * \ingroup COMPLETION
 */
int OSPD_Flush_group(OSP_group_t *group);

/**
 * \brief Reduce data from all processes and broadcast results to all processes.  
 *
 * \param[in] group          Group of processes.
 *
 * \see
 *
 * \ingroup MANYTOMANY
 */
int OSPD_Allreduce_group(OSP_group_t* group,
                       int count,
                       OSP_reduce_op_t osp_op,
                       OSP_datatype_t osp_type,
                       void* in,
                       void* out);

/**
 * \brief Reduce data from all processes and broadcast results to all processes.
 *
 * \param[in] group          Group of processes.
 *
 * \see
 *
 * \ingroup MANYTOMANY
 */
int OSPD_NbAllreduce_group(OSP_group_t* group,
                         int count,
                         OSP_reduce_op_t osp_op,
                         OSP_datatype_t osp_type,
                         void* in,
                         void* out,
                         OSP_handle_t osp_handle);

/**
 * \brief
 *
 * \param[in] group          Group of processes.
 *
 * \see
 *
 * \ingroup MANYTOMANY
 */
int OSPD_Bcast_group(OSP_group_t* group,
                   int root,
                   int count,
                   void* buffer);

/**
 * \brief
 *
 * \param[in] group          Group of processes.
 *
 * \see
 *
 * \ingroup MANYTOMANY
 */

int OSPD_NbBcast_group(OSP_group_t* group,
                      int root,
                      int count,
                      void* buffer,
                      OSP_handle_t osp_handle);

/**
 * \brief Device level implementation of OSP_Process_id 
 *
 * Returns process rank relative to the group base specified.
 *
 * \param[out] rc          Process id in the process group.
 * \param[in]  group       Process group.
 *
 * \ingroup INFORMATION
 */
int OSPD_Process_id(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_Process_total 
 *
 * Returns the total number of processes in  the group base specified.
 * 
 * \param[out] rc          Total number of processes in the process group.
 * \param[in]  group       Process group.
 *
 * \ingroup INFORMATION
 */
int OSPD_Process_total(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_Node_id
 *
 * Returns node rank relative to the group base specified.
 *
 * \param[out] rc          Node id in the process group.
 * \param[in]  group       Process group.
 *
 * \ingroup INFORMATION
 */
int OSPD_Node_id(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_Node_total
 *
 * Returns total number of nodes in the group base specified.
 * 
 * \param[out] rc          Total number of nodes in the process group.
 * \param[in]  group       Process group.
 *
 * \ingroup INFORMATION
 */
int OSPD_Node_total(OSP_group_t* group);

/**
 * \brief Device level implementation of OSP_Time_seconds 
 * 
 * Timer in units of seconds.
 *
 * \param[out] rc          Number of secs from an arbitrary time in the past.
 *
 * \ingroup INFORMATION
 */
double OSPD_Time_seconds(void);

/**
 * \brief Device level implementation of OSP_Time_cycles 
 *
 * Timer in units of cycles.
 *
 * \param[out] rc          Number of cycles from an arbitrary time in the past.
 *
 * \ingroup INFORMATION
 */
unsigned long long OSPD_Time_cycles(void);

void OSPD_Global_lock_acquire(void);

void OSPD_Global_lock_release(void);

#endif

#endif /* OSPD_H_INCLUDED */
