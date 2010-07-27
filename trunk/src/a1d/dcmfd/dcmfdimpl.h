/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <dcmf.h>
#include <dcmf_globalcollectives.h>
#include <dcmf_collectives.h>

#define A1C_REQUEST_POOL_INITIAL 1000
#define A1C_REQUEST_POOL_INCREMENT 100

#define A1DI_CRITICAL(call) do {                                  \
      if(A1D_Thread_info.thread_level > A1_THREAD_SERIALIZED) {   \
        DCMF_CriticalSection_enter(0);                            \
        call;                                                     \
        DCMF_CriticalSection_exit(0);                             \
      } else {                                                    \
        call;                                                     \
      }                                                           \
    } while (0)                                                   \

typedef struct
{
   uint32_t my_rank;
   uint32_t num_ranks;
}
A1D_Process_info_t;

typedef struct 
{
   int thread_level;
   int num_threads;
} 
A1D_Thread_info_t; 

/* TODO: Pack header supports only upto 3D arrays. Need to increase structure or 
 * find a better way to represent it */
typedef struct
{
   void *vaddress;
   uint32_t stride_levels;
   uint32_t src_stride_ar[2];
   uint32_t trg_stride_ar[2];
   uint32_t count[3];
} A1D_Pack_header_t;

typedef struct A1D_Request_info_t
{
   DCMF_Request_t request;
   uint32_t active;
   struct A1D_Request_info_t *next;
   struct A1D_Request_info_t *prev;
} A1D_Request_info_t;

typedef struct 
{
   A1D_Request_info_t *head;
   A1D_Request_info_t *tail;
   A1D_Request_info_t *current;
} A1D_Request_pool_t;

typedef struct
{
   DCMF_Protocol_t protocol;
   uint32_t rcv_active;
   void **xchange_ptr;
   uint32_t xchange_size;
} A1D_Control_xchange_info_t;

typedef struct
{
   DCMF_Protocol_t protocol;
   uint32_t rcv_active;
   DCMF_Control_t info;
} A1D_Control_fenceack_info_t;

typedef struct
{
   DCMF_Protocol_t protocol;
   uint32_t rcv_active;
} A1D_Send_info_t;

typedef struct
{
   DCMF_Protocol_t protocol;
   DCMF_Callback_t callback;
   uint32_t active;
} A1D_GlobalBarrier_info_t;

extern A1D_Thread_info_t A1D_Thread_info;
extern A1D_Process_info_t A1D_Process_info;
extern A1D_Control_xchange_info_t A1D_Control_xchange_info;
extern A1D_Control_fenceack_info_t A1D_Control_fenceack_info;
extern A1D_Send_info_t A1D_Send_noncontigput_info;
extern A1D_Send_info_t A1D_Send_fence_info;
extern A1D_GlobalBarrier_info_t A1D_GlobalBarrier_info;
extern A1D_Request_pool_t A1D_Request_pool;

extern DCMF_Configure_t A1D_Messager_info;
extern DCMF_Protocol_t A1D_Generic_put_protocol;
extern DCMF_Protocol_t A1D_Generic_get_protocol;
extern DCMF_Callback_t A1D_Nocallback;
extern DCMF_Memregion_t *A1D_Memregion_global;

extern void **A1D_Membase_global;

void A1DI_Generic_callback(void *, DCMF_Error_t *);

A1D_Request_info_t* A1DI_Get_request();
void A1DI_Free_request(A1D_Request_info_t *request);
