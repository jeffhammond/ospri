/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/** @file dcmfdimpl.h */

/*! \addtogroup osp OSPD dcmfd device interface
 * @{
 */

#include "osp.h"
#include "ospu.h"
#include "ospd.h"
#include <dcmf.h>
#include <dcmf_globalcollectives.h>
#include <dcmf_collectives.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <bpcore/bgp_atomic_ops.h>
#include <spi/bgp_SPI.h>

/*************************************************
 *                 Constants                     *
 ************************************************/

#define OSPC_ALIGNMENT 16

#define OSPC_ENABLE_CHT 1
#define OSPC_ENABLE_INTERRUPTS 0
#define OSPC_MPI_ACTIVE 1
#define OSPC_CHT_PAUSE_CYCLES 200

#define OSPC_PUT_PACKING_LIMIT 512
#define OSPC_GET_PACKING_LIMIT 512
#define OSPC_PUTACC_PACKING_LIMIT 2048

#define OSPC_PUT_PACKETSIZE 2048 
#define OSPC_GET_PACKETSIZE 2048 
#define OSPC_PUTACC_PACKETSIZE 8192 

#define OSPC_FLUSHALL_PENDING_LIMIT 512 

#define OSPC_REQUEST_POOL_SIZE 100 

#define OSPC_HANDLE_POOL_SIZE 100

#define OSPC_MAX_STRIDED_DIM 8

#define OSPC_USE_HANDOFF 1 

/* Currently we have two sizes of buffers to provide for Put/Get and
 * Acc packets. */
#define OSPC_BUFFER_SIZES 3

#define OSPC_PUT_BUFFERPOOL_SIZE 200;
#define OSPC_GET_BUFFERPOOL_SIZE 200;
#define OSPC_PUTACC_BUFFERPOOL_SIZE 200;

/*************************************************
*                  BGP Atomics                   *
*************************************************/

extern _BGP_Atomic global_atomic;

#define OSPDI_GLOBAL_ATOMIC_ACQUIRE()                 \
 {                                                   \
   volatile int done=0;                              \
   do {                                              \
     while(global_atomic.atom);                      \
     done = _bgp_test_and_set(&global_atomic, 1);    \
   } while(!done);                                   \
 }                                                   \

#define OSPDI_GLOBAL_ATOMIC_RELEASE() do{ global_atomic.atom = 0; _bgp_mbar(); }while(0)

/*************************************************
*                  Lockbox                       *
*************************************************/

extern LockBox_Mutex_t global_lbmutex;

/* Different cores which want to use independent lockbox mutexes should 
 * use different counters. So we try to find a free counter in a non-overlapping 
 * range of 200 counters. Counters range from 0-1023 */
#define OSPDI_GLOBAL_LBMUTEX_INITIALIZE()            	     	     \
 do {                                                  	     	     \
   int idx, coreid;                                   	     	     \
   coreid = Kernel_PhysicalProcessorID();                            \
   for(idx=200*coreid; idx<200*(coreid+1); idx++)          	     \
   {                                                         	     \
     if(!LockBox_AllocateMutex(idx, &global_lbmutex, coreid, 1, 0))  \
          break;					     	     \
   }       						             \
   OSPU_ERR_POP(idx == 200*(coreid+1),			 	     \
         "LockBox_AllocateMutex did not find a free index \n");      \
 } while(0)	                                                     \
       
#define OSPDI_GLOBAL_LBMUTEX_ACQUIRE() LockBox_MutexLock(global_lbmutex);

#define OSPDI_GLOBAL_LBMUTEX_RELEASE() LockBox_MutexUnlock(global_lbmutex);

/*************************************************
*           Lock Type Selection                  *
*************************************************/

#define OSPDI_GLOBAL_LOCK_ACQUIRE()     \
 do {                                  \
    if(!ospd_settings.mpi_active)       \
    {                                  \
         OSPDI_GLOBAL_ATOMIC_ACQUIRE(); \
    }                                  \
    else                               \
    {                                  \
        DCMF_CriticalSection_enter(0); \
    }                                  \
 } while(0);                           \

#define OSPDI_GLOBAL_LOCK_RELEASE()     \
 do {                                  \
    if(!ospd_settings.mpi_active)       \
    {                                  \
        OSPDI_GLOBAL_ATOMIC_RELEASE();  \
    }                                  \
    else                               \
    {                                  \
        DCMF_CriticalSection_exit(0);  \
    }                                  \
 } while(0);                           \

/*************************************************
*           Likely and Unlikely Ifs              *
*************************************************/

#define likely_if(x) if(__builtin_expect(x,1))
#define unlikely_if(x) if(__builtin_expect(x,0))

/*************************************************
 *          Generic  Macros                      *
 *************************************************/

#define OSPDI_Wait_cycles(cycles)                     \
   do {                                              \
      unsigned long long start = DCMF_Timebase();    \
      while((DCMF_Timebase() - start) < cycles);     \
   } while(0)                                        \

#define OSPDI_Wait_seconds(seconds)               \
   do {                                           \
      double start = DCMF_Timer();               \
      while((DCMF_Timer() - start) < seconds);   \
   } while(0)                                    \

#define OSPDI_Set_handle(request, handle)  \
do {                                      \
    request->handle_ptr = handle;         \
   } while(0)                             \

/*************************************************
 *          Memory Allocation Macros             *
 *************************************************/

#define OSPDI_Malloc(ptr, num) posix_memalign(ptr, ospd_settings.alignment, num)
/*
 * I don't know why one would want unaligned memory, but here it is for posterity
 * #define OSPDI_Malloc(ptr, num)  ((ptr = malloc(num)) == NULL)
 */

#define OSPDI_Free(ptr) free(ptr)

#define OSPDI_Memset(ptr, val, num)  memset(ptr, val, num)

#define OSPDI_Memcpy(trg, src, num)  memcpy(trg, src, num)

/*************************************************
 *          Critical Section Macros              *
 *************************************************/

/**
 * \brief Handles non-contiguous puts which have been handed-off to the CHT.
 *
 * \see OSPD_NbPutS, OSPDI_CHT_advance_lock
 *
 * \ingroup CHT
 */
void OSPDI_Handoff_progress();

#define OSPDI_CRITICAL_ENTER()                                     \
    do {                                                          \
      if(ospd_settings.enable_cht || ospd_settings.mpi_active)      \
      {                                                           \
        OSPDI_GLOBAL_LOCK_ACQUIRE();                               \
      }     							  \
    } while (0)                                                   \

#define OSPDI_CRITICAL_EXIT()                                      \
    do {                                                          \
      if(ospd_settings.enable_cht || ospd_settings.mpi_active)      \
      {                                                           \
        OSPDI_GLOBAL_LOCK_RELEASE();                               \
      }                                                           \
    } while (0)                                                   \

#define OSPDI_Advance()                                             \
 do {                                                              \
         DCMF_Messager_advance(0);                                 \
         if (ospd_settings.use_handoff && (OSPD_Inside_handoff==0))  \
         {        						   \
             OSPDI_Handoff_progress();                              \
         }                                                         \
    } while(0)                                                     \

#define OSPDI_Conditional_advance(boolean)                           \
    while(boolean) {                                                \
          DCMF_Messager_advance(0);                                 \
          if (ospd_settings.use_handoff && (OSPD_Inside_handoff==0))  \
          {							    \
                OSPDI_Handoff_progress();                            \
          }							    \
    }                                                               \

/*************************************************
 *          Computation macros                   *
 *************************************************/

#define OSPDI_ACC(datatype, source, target, scaling, count)                  \
   do {                                                                     \
     int i;                                                                 \
     datatype *s = (datatype *) source;                                     \
     datatype *t = (datatype *) target;                                     \
     datatype c = (datatype) scaling;                                       \
     for(i=0; i<count; i++)                                                 \
          t[i] += s[i]*c;                                                   \
   } while(0)                                                               \

#define OSPDI_MOD_BXOR(datatype, source, target, count)                      \
   do {                                                                     \
     int i;                                                                 \
     datatype *s = (datatype *) source;                                     \
     datatype *t = (datatype *) target;                                     \
     for(i=0; i<count; i++)                                                 \
          t[i] ^= s[i];                                                     \
   } while(0)                                                               \

 /* TODO probably need to optimize these functions for double-hummer */
#define OSPDI_ACC_DOUBLE(source, target, scaling, count)                  \
   do {                                                                     \
     int i;                                                                 \
     double *s = (double *) source;                                     \
     double *t = (double *) target;                                     \
     double c = (double) scaling;                                       \
     for(i=0; i<count; i++)                                                 \
          t[i] += s[i]*c;                                                   \
   } while(0)                                                               \

#define OSPDI_ABS(datatype, source, target, count)                           \
   do {                                                                     \
     int i;                                                                 \
     datatype *s = (datatype *) source;                                     \
     datatype *t = (datatype *) target;                                     \
     for(i=0; i<count; i++) t[i] = ( s[i] > 0 ? s[i] : -s[i]);              \
   } while(0)                                                               \

/* NOTE: fabs will compile to the best assembly. */
 #define OSPDI_ABS_DOUBLE(source, target, count)                           \
    do {                                                                  \
      int i;                                                              \
      double *s = (double *) source;                                     \
      double *t = (double *) target;                                     \
      for(i=0; i<count; i++) t[i] = fabs(s[i]);                            \
    } while(0)                                                             \

#define OSPDI_FETCHANDADD_EXECUTE(datatype, source, target, original, count) \
   do {                                                                     \
     int i;                                                                 \
     datatype *s = (datatype *) source;                                     \
     datatype *t = (datatype *) target;                                     \
     datatype *o = (datatype *) original;                                   \
     for(i=0; i<count; i++)                                                 \
     {                                                                      \
          o[i] = t[i];                                                      \
          t[i] += s[i];                                                     \
     }                  						                            \
   } while(0)                                                              \

/*************************************************
 *             Data Structures                   *
 *************************************************/
typedef enum
{
  OSPD_MUTEX_LOCK = 0,
  OSPD_MUTEX_TRYLOCK,
  OSPD_MUTEX_UNLOCK
} OSPD_Mutex_op_type;

typedef struct
{
    volatile uint32_t enable_cht;
    volatile uint32_t mpi_active;
    volatile uint32_t cht_pause_cycles;
    volatile uint32_t enable_interrupts;
    volatile uint32_t put_packing_limit;
    volatile uint32_t get_packing_limit;
    volatile uint32_t putacc_packing_limit;
    volatile uint32_t put_packetsize;
    volatile uint32_t get_packetsize;
    volatile uint32_t putacc_packetsize;
    volatile uint32_t flushall_pending_limit;
    volatile uint32_t alignment;
    volatile uint32_t handlepool_size;
    volatile uint32_t requestpool_size;
    volatile uint32_t put_bufferpool_size;
    volatile uint32_t get_bufferpool_size;
    volatile uint32_t putacc_bufferpool_size;
    volatile uint32_t use_handoff;
} OSPD_Settings_t;

typedef struct
{
    size_t my_rank;
    size_t my_node;
    size_t num_ranks;
    size_t num_nodes;
    DCMF_Hardware_t hw;
} OSPD_Process_info_t;

typedef struct
{
    int   rank;
    long  *value_ptr; /*This will hold ptr it value if counter
                        is located on a remote process*/
    long  value;      /*This will contain the value if counter
                        is located locally*/
} OSPD_Counter_t;

typedef struct OSPD_Mutex_request_t
{
  int rank;
  struct OSPD_Mutex_request_t *next;
} OSPD_Mutex_request_t;

typedef struct
{
  int   mutex;
  OSPD_Mutex_request_t *head;
  OSPD_Mutex_request_t *tail;
} OSPD_Mutex_t;

typedef struct OSPD_Buffer_t
{
  void *buffer_ptr;
  int pool_index;
  struct OSPD_Buffer_t *next;
} OSPD_Buffer_t;

typedef struct
{
  OSPD_Buffer_t* pool_heads[OSPC_BUFFER_SIZES];
  int limits[OSPC_BUFFER_SIZES];
  int sizes[OSPC_BUFFER_SIZES];
  void* pool_region_ptrs[OSPC_BUFFER_SIZES];
  void* mem_region_ptrs[OSPC_BUFFER_SIZES];
} OSPD_Buffer_pool_t;

typedef struct OSPD_Handle_t
{
    volatile int active;
    volatile int active_list_index;
    struct OSPD_Handle_t *next;
} OSPD_Handle_t;

typedef struct OSPD_Handle_pool_t
{
    OSPD_Handle_t *head;
    void *region_ptr;
} OSPD_Handle_pool_t;

typedef struct OSPD_Request_t
{
    DCMF_Request_t request;
    int in_pool;
    void* buffer_ptr;
    OSPD_Buffer_t *ospd_buffer_ptr;
    uint32_t buffer_size;
    OSPD_Handle_t *handle_ptr;
    struct OSPD_Request_t *next;
} OSPD_Request_t;

typedef struct
{
    OSPD_Request_t *head;
    OSPD_Request_t *region_ptr;
} OSPD_Request_pool_t;

typedef struct
{
    DCMF_Protocol_t protocol;
    volatile int rcv_active;
    void **xchange_ptr;
    size_t xchange_size;
} OSPD_Control_xchange_info_t;

typedef union
{
    DCQuad info[2];
    struct
    {
        void* target_ptr;
        OSP_datatype_t datatype;
        union
        {
            int32_t int32_value;
            int64_t int64_value;
            uint32_t uint32_value;
            uint64_t uint64_value;
            float float_value;
            double double_value;
        } scaling;
    };
} OSPD_Putacc_header_t;

typedef union
{
    DCQuad info[2];
    struct
    {
        void* target_ptr;
        OSP_reduce_op_t op;
        OSP_datatype_t datatype;
    };
} OSPD_Putmod_header_t;

typedef struct
{
    int stride_level;
    int block_sizes[OSPC_MAX_STRIDED_DIM];
    void *target_ptr;
    int trg_stride_ar[OSPC_MAX_STRIDED_DIM-1];
    int block_idx[OSPC_MAX_STRIDED_DIM];
    int data_size; 
} OSPD_Packed_puts_header_t;

typedef struct
{
    int stride_level;
    int block_sizes[OSPC_MAX_STRIDED_DIM];
    void *target_ptr;
    int trg_stride_ar[OSPC_MAX_STRIDED_DIM-1];
    int block_idx[OSPC_MAX_STRIDED_DIM];
    int data_size;
    OSPD_Handle_t *handle_ptr;
} OSPD_Packed_gets_response_header_t;

typedef struct
{
    uint32_t target;
    int stride_level;
    int block_sizes[OSPC_MAX_STRIDED_DIM];
    void* source_ptr;
    int src_stride_ar[OSPC_MAX_STRIDED_DIM-1];
    void* target_ptr;
    int trg_stride_ar[OSPC_MAX_STRIDED_DIM-1];
    OSPD_Handle_t *handle_ptr;
} OSPD_Packed_gets_header_t;

typedef union
{
   DCQuad info[2];
   struct
   {
     int bytes;
     int source;
     void* source_ptr_out; 
     void* target_ptr;
     OSP_atomic_op_t op;
     OSP_datatype_t datatype;
     OSPD_Handle_t* handle_ptr;
   };
} OSPD_Rmw_header_t;

typedef struct
{
   int bytes;
   void* source_ptr_out; 
   OSPD_Handle_t* handle_ptr;
} OSPD_Rmw_response_header_t;

typedef struct
{
    long *value_ptr;
    long value;
} OSPD_Counter_pkt_t;

typedef struct
{
    int mutex_idx;
    OSPD_Mutex_op_type mutex_op;
    int response;
} OSPD_Mutex_pkt_t;

typedef struct
{
    int stride_level;
    int block_sizes[OSPC_MAX_STRIDED_DIM];
    void *target_ptr;
    int trg_stride_ar[OSPC_MAX_STRIDED_DIM-1];
    int block_idx[OSPC_MAX_STRIDED_DIM];
    int data_size;
    OSP_datatype_t datatype;
    union
    {
        int32_t int32_value;
        int64_t int64_value;
        uint32_t uint32_value;
        uint64_t uint64_value;
        float float_value;
        double double_value;
    } scaling;
} OSPD_Packed_putaccs_header_t;


/*************************************************
 *             Global variables                  *
 ************************************************/

extern pthread_t OSPDI_CHT_pthread;

extern OSPD_Process_info_t OSPD_Process_info;
extern OSPD_Control_xchange_info_t OSPD_Control_xchange_info;
extern OSPD_Request_pool_t OSPD_Request_pool;
extern OSPD_Handle_pool_t OSPD_Handle_pool;
extern OSPD_Buffer_pool_t OSPD_Buffer_pool;

extern int *OSPD_Mutexes_count;
extern OSPD_Mutex_t *OSPD_Mutexes;

extern DCMF_Configure_t OSPD_Messager_info;
extern DCMF_Protocol_t OSPD_Control_flushack_protocol;
extern DCMF_Protocol_t OSPD_Send_flush_protocol;
extern DCMF_Protocol_t OSPD_GlobalBarrier_protocol;
extern DCMF_CollectiveProtocol_t OSPD_GlobalAllreduce_protocol;
extern DCMF_Protocol_t OSPD_GlobalBcast_protocol;
extern DCMF_Protocol_t OSPD_Generic_put_protocol;
extern DCMF_Protocol_t OSPD_Generic_get_protocol;
extern DCMF_Protocol_t OSPD_Generic_putacc_protocol;
extern DCMF_Protocol_t OSPD_Generic_putmod_protocol;
extern DCMF_Protocol_t OSPD_Rmw_protocol;
extern DCMF_Protocol_t OSPD_Rmw_response_protocol;
extern DCMF_Protocol_t OSPD_Packed_puts_protocol;
extern DCMF_Protocol_t OSPD_Packed_gets_protocol;
extern DCMF_Protocol_t OSPD_Packed_gets_response_protocol;
extern DCMF_Protocol_t OSPD_Packed_putaccs_protocol;
extern DCMF_Protocol_t OSPD_Counter_create_protocol;
extern DCMF_Protocol_t OSPD_Counter_protocol;
extern DCMF_Protocol_t OSPD_Mutex_protocol;
extern DCMF_Protocol_t OSPD_Control_protocol;
extern DCMF_Callback_t OSPD_Nocallback;
extern DCMF_Memregion_t *OSPD_Memregion_global;

extern void **OSPD_Membase_global;
extern void **OSPD_Put_Flushcounter_ptr;
extern OSPD_Handle_t **OSPD_Active_handle_list;
extern volatile int *OSPD_Connection_send_active;
extern volatile int *OSPD_Connection_put_active;
extern volatile int OSPD_Control_flushack_active;
extern volatile int OSPD_Put_flushack_active;
extern volatile int OSPD_Inside_handoff;

extern OSPD_Settings_t ospd_settings;

/************************************************* 
 *             Function Prototypes               *
 ************************************************/

void *OSPDI_CHT_advance_lock(void *);

void OSPDI_Global_lock_acquire();

void OSPDI_Global_lock_release();

void OSPDI_Generic_done(void *, DCMF_Error_t *);

void OSPDI_Request_done(void *, DCMF_Error_t *);

int OSPDI_Memregion_Global_initialize();

int OSPDI_Put_initialize();

int OSPDI_Packed_puts_initialize();

int OSPDI_Get_initialize();

int OSPDI_Rmw_initialize();

int OSPDI_Counter_initialize();

int OSPDI_Request_pool_initialize();

void OSPDI_Request_pool_finalize();

OSPD_Request_t* OSPDI_Get_request(int);

void OSPDI_Release_request(OSPD_Request_t *);

int OSPDI_Buffer_pool_initialize();

void OSPDI_Buffer_pool_finalize();

OSPD_Buffer_t* OSPDI_Get_buffer(int, int);

void OSPDI_Release_buffer(OSPD_Buffer_t *);

int OSPDI_Handle_pool_initialize();

void OSPDI_Handle_pool_finalize();

OSPD_Handle_t* OSPDI_Get_handle();

void OSPDI_Release_handle(OSPD_Handle_t *);

int OSPDI_Packed_gets_initialize();

int OSPDI_Putacc_initialize();

int OSPDI_Putmod_initialize();

int OSPDI_Packed_putaccs_initialize();

int OSPDI_GlobalBarrier_initialize();

int OSPDI_GlobalAllreduce_initialize();

int OSPDI_GlobalBcast_initialize();

int OSPDI_Send_flush_initialize();

int OSPDI_Put_flush_initialize();

int OSPDI_Control_flushack_initialize();

int OSPDI_GlobalBarrier();

int OSPDI_Read_parameters();

int OSPDI_Send_flush(int proc);

int OSPDI_Pack_strided(void *packet_ptr,
                      int packet_limit,
                      int stride_level,
                      int *block_sizes,
                      void **source_ptr,
                      int *src_stride_ar,
                      void **target_ptr,
                      int *trg_stride_ar,
                      int *block_idx,
                      int *data_size,
                      int *complete);

int OSPDI_Unpack_strided(void *packet_ptr,
                        int data_size,
                        int stride_level,
                        int *block_sizes,
                        void *target_ptr,
                        int *trg_stride_ar,
                        int *block_idx,
                        int *complete);

int OSPDI_Unpack_strided_acc(void *data_ptr,
                            int data_size,
                            int stride_level,
                            int *block_sizes,
                            void *target_ptr,
                            int *trg_stride_ar,
                            int *block_idx,
                            OSP_datatype_t osp_type,
                            void *scaling,
                            int *complete);

/*****************************************************
                 Packing Handoff
*****************************************************/

typedef enum
{
  OSPD_PACKED_PUTS = 0,
  OSPD_PACKED_PUTACCS
} OSPD_Op_type;

typedef struct
{
   int target;
   int stride_level;
   int *block_sizes;
   void *source_ptr;
   int *src_stride_ar;
   void *target_ptr;
   int *trg_stride_ar;
   OSPD_Handle_t *ospd_handle;
} OSPD_Puts_op;

typedef struct
{
   int target;
   int stride_level;
   int *block_sizes;
   void *source_ptr;
   int *src_stride_ar;
   void *target_ptr;
   int *trg_stride_ar;
   OSP_datatype_t datatype;
   void *scaling;
   OSPD_Handle_t *ospd_handle;
} OSPD_Putaccs_op;

typedef struct OSPD_Op_handoff
{
   OSPD_Op_type op_type;
   union
   { 
     OSPD_Puts_op puts_op;
     OSPD_Putaccs_op putaccs_op;
   } op;  
   void *op_ptr;
   struct OSPD_Op_handoff *next;
} OSPD_Op_handoff;

extern OSPD_Op_handoff *OSPD_Op_handoff_queuehead;
extern OSPD_Op_handoff *OSPD_Op_handoff_queuetail;

/*! @} */
