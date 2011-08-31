/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#if !defined OSP_H_INCLUDED
#define OSP_H_INCLUDED

/* Keep C++ compilers from getting confused */
#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* needed for uint64_t etc. */
#include <stdint.h>

    /** @file osp.h.in */

    /*! \addtogroup osp OSP Public Interface
     * @{
     */

    /* ********************************************************************* */
    /*                                                                       */
    /*               Enumerations                                            */
    /*                                                                       */
    /* ********************************************************************* */

    /**
     * \brief Thread support within OSP.
     *
     * Implementation details:
     *
     * OSP_THREAD_SINGLE, OSP_THREAD_FUNNELED, OSP_THREAD_SERIALIZED and
     * OSP_THREAD_MULTIPLE have the same meaning as the MPI descriptors
     * of similar name.
     *
     * \ingroup ENUMS
     *
     */

    typedef enum
    {
        OSP_THREAD_SINGLE,     /**< There is only one thread present. */
        OSP_THREAD_FUNNELED,   /**< There are multiple threads present
                                but only the main thread makes OSP calls. */
        OSP_THREAD_SERIALIZED, /**< There are multiple threads present and each can make OSP calls,
                                but never simultaneously. */
        OSP_THREAD_MULTIPLE    /**< There are multiple threads present and each can make OSP calls. */
    }
    osp_thread_level;

    /**
     * \brief Datatype support within OSP.
     *
     * \note OSP does not support complex numbers yet.  This is okay because
     *       imaginary numbers don't exist, by definition :-)
     *
     * \ingroup ENUMS
     *
     */

    typedef enum
    {
        OSP_INT32,  /**< int32  */
        OSP_INT64,  /**< int64  */
        OSP_UINT32, /**< uint32 */
        OSP_UINT64, /**< uint64 */
        OSP_FLOAT,  /**< single-precision */
        OSP_DOUBLE, /**< double-precision */
    }
    osp_datatype_t;

    /**
     * \brief OSP boolean.
     *
     * \ingroup ENUMS
     *
     */

    typedef enum
    {
        OSP_TRUE = 1,
        OSP_FALSE = 0,
    }
    osp_bool_t;

    /**
     * \brief OSP result (error code).
     *
     * \ingroup ENUMS
     *
     * \see OSP_Translate_result
     *
     */

    typedef enum
    {
        OSP_SUCCESS = 0,
        OSP_UNKNOWN_ERROR,
        OSP_OUT_OF_MEMORY_ERROR,
        OSP_OTHER_ALLOCATION_ERROR,
        OSP_FATAL_NETWORK_ERROR,
        OSP_RECOVERABLE_NETWORK_ERROR,
        OSP_INVALID_GROUP,
        OSP_INVALID_HANDLE,
    }
    osp_result_t;

    /**
     * \brief OSP reduction operations
     *
     * \ingroup ENUMS
     *
     */

    typedef enum
    {
        OSP_SUM,
        OSP_PROD,
        OSP_MAX,
        OSP_MIN,
        OSP_MAXABS, /* Returns MAX(ABS(buffer)) */
        OSP_MINABS, /* Returns MIN(ABS(buffer)) */
        OSP_OR,     /* OR - only for integer types */
        OSP_XOR,    /* XOR - only for integer types */
        OSP_BOR,    /* bit-wise OR - only for integer types */
        OSP_BXOR,   /* bit-wise XOR - only for integer types */
        OSP_SAME,   /* Sets the return value if the buffer (element) is the same across all nodes.
                  This operation is equivalent to doing MAX(buffer) and MIN(-buffer) simultaneously
                  and testing for equality element-wise. */
    }
    osp_reduce_op_t;

    /**
     * \brief OSP RMW operations
     *
     * \ingroup ENUMS
     *
     */

    typedef enum
    {
        OSP_INC, /* x += 1 */
        OSP_ADD, /* x += y */
        OSP_AND, /* x &= y */
        OSP_OR,  /* x |= y */
        OSP_XOR, /* x ^= y */
        OSP_FETCH_AND_INC, /* get x; x += 1 */
        OSP_FETCH_AND_ADD, /* get x; x += y */
        OSP_FETCH_AND_AND, /* get x; x &= y */
        OSP_FETCH_AND_OR,  /* get x; x |= y */
        OSP_FETCH_AND_XOR, /* get x; x ^= y */
        OSP_COMPARE_AND_SWAP,
        OSP_SWAP,
    }
    osp_atomic_op_t;

    /* ********************************************************************* */
    /*                                                                       */
    /*               OSP data structures                                      */
    /*                                                                       */
    /* ********************************************************************* */

    /**
     * \brief OSP locale type.
     *
     * Like an endpoint.
     *
     * \see
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef uint32_t osp_locale_t;

    /**
     * \brief OSP offset type.
     *
     * \see OSP_PutMR
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef uint64_t osp_offset_t;

    /**
     * \brief OSP device type.
     *
     * A device which has its own address space.
     *
     * \see OSP_Allocate_global_memregion, OSP_Destroy_global_memregion
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_device
    {
        char name[];
        void* ospd_device_info; /* implementation-specific information */
    }
    osp_device_t;

    /**
     * \brief OSP global memregion type.
     *
     * A global memregion is like an MPI-2 RMA Window or a stripped-down GA global array.
     * It is an opaque object which holds all the necessary information required to
     * perform RMA operations.
     *
     * The contents of the this type are the local and global info.
     * ospd_local_memregion_info contains either the native aka CPU info,
     * meaning the base pointer and total size, or it contains the
     * device pointer and total size if referring to GPU memory.
     * Obviously, whether the memory is in the same address space
     * or an attached address space must be specified.
     *
     * The global memregion info is where we hide the details of remote registration.
     * For symmetric allocation, it can be O(1) elements whereas the more common
     * case requires O(N) elements.
     *
     * \see OSP_Create_global_memregion, OSP_Allocate_global_memregion, OSP_Destroy_global_memregion
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_global_memregion
    {
        void* ospd_local_memregion_info;  /**/
        void* ospd_global_memregion_info; /**/
    }
    osp_global_memregion_t;

    /**
     * \brief OSP non-blocking handle type.
     *
     * \see OSP_Test_handle, OSP_Test_handle_list,
     *      OSP_Wait_handle, OSP_Wait_handle_list,
     *      OSP_Reset_handle, OSP_Reset_handle_list
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef void* osp_handle_t;

    /**
     * \brief OSP shared counter.
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_counter
    {
        osp_locale_t host_rank;
        void* ospd_counter_info;
    }
    osp_counter_t;

    /**
     * \brief OSP mutex.
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_mutex
    {
        osp_locale_t host_rank;
        void* ospd_mutex_info;
    }
    osp_mutex_t;

    /**
     * \brief OSP stride descriptor type.
     *
     * \see OSP_PutS, OSP_GetS, OSP_AccS, etc.
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_stride_descr
    {
        int num_strides;
        int* stride_offset;
        int* block_size;
    }
    osp_stride_descr_t;

    /**
     * \brief OSP accumulate descriptor type.
     *
     * \see OSP_PutS, OSP_GetS, OSP_AccS, etc.
     *
     * \ingroup TYPEDEFS
     *
     */

    typedef struct osp_acc_descr
    {
        osp_datatype_t type;
        void* scaling_factor;
    }
    osp_acc_descr_t;

    /* ********************************************************************* */
    /*                                                                       */
    /*               OSP external API - Helper Functions                      */
    /*                                                                       */
    /* ********************************************************************* */

    /**
     * \brief Timer in units of cycles.
     *
     * \warning This function is not guaranteed to produce correct results for
     *          arbitrarily-long durations (e.g. in the case where the underlying
     *          cycle-accurate counter outputs a 32-bit integer).
     *
     * \param[out] rc   Number of cpu clock cycles from an arbitrary time in the past.
     *
     * \see OSP_Time_seconds
     *
     * \ingroup INFORMATION
     */

    unsigned long long OSP_Time_cycles(void);

    /**
     * \brief Timer in units of seconds.
     *
     *
     * \param[out] rc   Number of seconds from an arbitrary time in the past.
     *
     * \see OSP_Time_cycles
     *
     * \ingroup INFORMATION
     */

    double OSP_Time_seconds(void);

    /**
     * \brief Prints a translation of the error code enum.  Silent for OSP_SUCCESS.
     *
     * This function can be implemented as a macro instead.
     *
     * \param[int] result   Return code to translate.
     *
     * \see osp_result_t
     *
     * \ingroup INFORMATION
     */

    void OSP_Translate_errors(osp_result_t result);

    /* ********************************************************************* */
    /*                                                                       */
    /*               OSP external API - Process Information                   */
    /*                                                                       */
    /* ********************************************************************* */

    /* we're using MPI for this */

    /* ********************************************************************* */
    /*                                                                       */
    /*               OSP external API - management                            */
    /*                                                                       */
    /* ********************************************************************* */

    /**
     * \brief Initializes the OSPRI environment.
     *
     * \warning If the MPID device is used, one must call MPI_Init/MPI_Init_thread prior to OSP_Initialize.
     *          The implementation shall verify this.
     *
     * \note This routine is called OSP_Initialize for syntactic symmetry with
     *       OSP_Finalize and because of Jeff's schadenfreude toward Pavan.
     *
     * \param[out] rc               The error code from initializing OSP
     * \param[in]  osp_thread_level  The type of thread support for OSP
     *
     * \see osp_thread_level
     *
     * \ingroup MANAGEMENT
     */

    osp_result_t OSP_Initialize(int osp_thread_level);

    /**
     * \brief Terminates the OSP environment normally.
     *
     * \warning If the MPID device is used, one should be careful in using OSP_Finalize and MPI_Finalize.
     *
     * \note Unlike MPI, OSP_Finalize may be called many times and is thread-safe.
     *       Improper use of OSP_Finalize (calling it more than once) shall be
     *       indicated by a non-zero error code and potentially warnings in
     *       stderr.
     *
     * \note Implementation detail: we need global variable indicated where or not
     *       OSP is alive or not to make this function thread-safe.
     *
     * \param[out] rc    The error code from terminating OSP.
     *
     * \see OSP_Initialize, OSP_Abort
     *
     * \ingroup MANAGEMENT
     */

    osp_result_t OSP_Finalize(void);

    /**
     * \brief Terminates the OSP environment abnormally.
     *
     * \warning This must be the last OSP call made in any code unless OSP_Finalize is called instead.
     *
     * \param[in]  error_code    The error code to be returned to the submitting environment.
     * \param[in]  error_message Text string to print to stderr upon termination.
     *
     * \see OSP_Initialize, OSP_Finalize
     *
     * \ingroup MANAGEMENT
     */

    void OSP_Abort(int error_code, char error_message[]);

    /* ********************************************************************* */
    /*                                                                       */
    /*               OSP external API - memory                                */
    /*                                                                       */
    /* ********************************************************************* */

    /**
     * \brief A collective operation to allocate memory to be used in context of OSP copy operations.
     *
     * \note With respect to devices in the attached-processor hybrid model, we consider attached processors
     *       to be part of the same rank as the process that controls them.
     *
     * \note Memory allocated with this function will be properly aligned for the architecture.
     *
     * \warning Memory allocated with this function must be freed by OSP_Destroy_global_memregion.
     *
     * \param[out] rc            The error code.
     * \param[in]  comm          MPI communicator over which the memregion is shared.
     * \param[in]  bytes         The amount of memory provided by ptr.
     * \param[in]  device        The device address space in which the memregion is created.
     * \param[out] memregion     Pointer to memregion structure to be created.
     *
     * \see OSP_Create_global_memregion, OSP_Destroy_global_memregion
     *
     * \ingroup MEMORY
     */

    osp_result_t OSP_Allocate_global_memregion(MPI_Comm comm,
                                               osp_offset_t bytes,
                                               osp_device_t device,
                                               osp_global_memregion_t* memregion);

    /**
     * \brief A collective operation to register previously allocated memory to be used in context of OSP copy operations.
     *
     * \note Memory allocated with this function will be properly aligned for the architecture.
     *
     * \warning Memory allocated with this function must be freed by OSP_Destroy_global_memregion.
     *
     * \param[out] rc            The error code.
     * \param[in]  group         OSP group over which the memregion is shared.
     * \param[in]  bytes         The amount of memory provided by ptr.
     * \param[in]  ptr           Pointer to memory to be included in the memregion .
     * \param[out] memregion     Pointer to memregion structure to be created.
     *
     * \see OSP_Allocate_global_memregion, OSP_Destroy_global_memregion
     *
     * \ingroup MEMORY
     */

    osp_result_t OSP_Create_global_memregion(MPI_Comm comm,
                                             osp_offset_t bytes,
                                             void* ptr,
                                             osp_global_memregion_t* memregion);

    /**
     * \brief A collective operation (over the group associated with the memregion)
     *        to free memory allocated by OSP_Create_global_memregion.
     *
     * \param[out] rc            The error code.
     * \param[in]  memregion     Pointer to memregion structure to be destroyed.
     *
     * \see OSP_Allocate_global_memregion, OSP_Create_global_memregion
     *
     * \ingroup MEMORY
     */

    osp_result_t OSP_Destroy_global_memregion(osp_global_memregion_t* memregion);


    /**
     * \brief Initializes the give non-blocking handle.
     *
     * \param[in] handle      Non-blocking handle upon which to be waited.
     *
     * \see osp_handle_t, OSP_Wait_handle_list, OSP_Test_handle
     *
     * \ingroup MEMORY
     */

    osp_result_t OSP_Allocate_handle(osp_handle_t *handle);

    /**
     * \brief Initializes the give non-blocking handle.
     *
     * \param[in] handle      Non-blocking handle upon which to be waited.
     *
     * \see osp_handle_t, OSP_Wait_handle_list, OSP_Test_handle
     *
     * \ingroup MEMORY
     */

    osp_result_t OSP_Release_handle(osp_handle_t handle);

    /* ********************************************************************** */
    /*                                                                        */
    /*               OSP external API - atomic operations                      */
    /*                                                                        */
    /* ********************************************************************** */

    /**
     * \brief Collective operation to allocate and register a counter.
     *        After this operation returns, the shared counter is visible
     *        from every process in the group.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the counter is shared.
     * \param[inout]  counter       OSP shared counter.
     *
     * \see osp_counter_t, OSP_Destroy_counter, OSP_Incr_counter, OSP_NbIncr_counter
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Create_counter(MPI_Comm comm, osp_counter_t* counter);

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

    osp_result_t OSP_Destroy_counter(MPI_Comm comm, osp_counter_t* counter);

    /**
     * \brief Atomically updates a shared counter and returns the current value.
     *
     * \param[out] rc            The error code.
     * \param[in]  group         OSP group over which the counter is shared.
     * \param[in]  counter       OSP shared counter.
     * \param[in]  increment     The value to add to the counter.
     * \param[in]  original      The remote value of the counter prior to the increment.
     *
     * \see osp_counter_t, OSP_Create_counter, OSP_Destroy_counter, OSP_NbIncr_counter
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Incr_counter(MPI_Comm comm,
            osp_counter_t* counter,
            long  increment,
            long* original);

    /**
     * \brief Non-blocking atomic RMW update of a shared counter.
     *
     * \param[out] rc            The error code.
     * \param[in]  group         OSP group over which the counter is shared.
     * \param[in]  counter       OSP shared counter.
     * \param[in]  increment     The value to add to the counter.
     * \param[in]  original      The remote value of the counter prior to the increment.
     * \param[out] handle        Opaque handle for the request
     *
     * \see osp_counter_t, OSP_Create_counter, OSP_Destroy_counter, OSP_Incr_counter
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_NbIncr_counter(MPI_Comm comm,
            OSP_counter_t counter,
            long  increment,
            long* original,
            osp_handle_t handle);

    /**
     * \brief Collective operation to initialize a list of mutexes.
     *
     * The allocation of
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     mutex_count   Number of mutexes to be created.
     * \param[int]    mutex_array   An arrays storing the number of mutexes on each process
     *
     * \see osp_mutex_t, OSP_Destroy_mutexes, OSP_Lock_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Create_mutexes(MPI_Comm comm,
            int mutex_count,
            osp_mutex_t* *mutex_array);

    /**
     * \brief Collective operation to uninitialize a list of mutexes
     *
     * \note Implementors: OSP_mutex_t cannot be dynamically allocated otherwise this function
     *                     will create a memory leak.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     mutexes       Pointer to vector OSP mutexes.
     *
     * \see osp_mutex_t, OSP_Create_mutexes, OSP_Lock_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Destroy_mutexes(MPI_Comm comm,
            int mutex_count,
            osp_mutex_t* *mutex_array);

    /**
     * \brief Local operation to lock a mutex.  This call blocks until the mutex has been locked.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     mutex         OSP mutex.
     * \param[in]     proc          Process on which you want to lock mutex on
     *
     * \see osp_mutex_t, OSP_Trylock_mutex, OSP_Unlock_mutex, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Lock_mutex(MPI_Comm comm,
            osp_mutex_t* mutex);

    /**
     * \brief Local operation to trylock a mutex.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     mutex         OSP mutex.
     * \param[in]     proc          Process on which you want to lock mutex on
     *
     * \see osp_mutex_t, OSP_Lock_mutex, OSP_Unlock_mutex, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Trylock_mutex(MPI_Comm comm,
            osp_mutex_t* mutex,
            osp_bool_t *acquired);

    /**
     * \brief Local operation to unlock a mutex.  This call blocks until the mutex has been unlocked.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     mutex         OSP mutex.
     * \param[in]     proc          Process on which you want to lock mutex on
     *
     * \see osp_mutex_t, OSP_Lock_mutex, OSP_Trylock_mutex, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Unlock_mutex(MPI_Comm comm,
            osp_mutex_t* mutex);

    /**
     * \brief Local operation to lock a list of mutexes.  This call blocks until ALL
     *        mutexes have been locked.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     count         Number of mutexes to be locked.
     * \param[in]     mutexes       Pointer to vector OSP mutexes.
     * \param[in]     proc          Pointer to vector or process ranks.
     *
     * \see osp_mutex_t, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Lock_mutexes(MPI_Comm comm,
            int count,
            osp_mutex_t* *mutex_array);

    /**
     * \brief Local operation to trylock a list of mutexes.  This call attempts to acquire all the
     *        locks given and sets the value of the corresponding offset in a vector of booleans
     *        indicating if the lock was acquired or not.
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     count         Number of mutexes to be locked.
     * \param[in]     mutexes       Pointer to vector OSP mutexes.
     * \param[inout]  acquired      Pointer to vector OSP booleans indicating if the lock was acquired.
     *
     * \see osp_mutex_t, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Trylock_mutexes(MPI_Comm comm,
            int count,
            osp_mutex_t* *mutex_array,
            osp_bool_t* *acquired);

    /**
     * \brief Local operation to unlock a list of mutexes
     *
     * \param[out]    rc            The error code.
     * \param[in]     group         OSP group over which the mutexes are shared.
     * \param[in]     count         Number of mutexes to be locked.
     * \param[in]     mutexes       Pointer to vector OSP mutexes.
     *
     * \see osp_mutex_t, OSP_Create_mutexes, OSP_Destroy_mutex, OSP_Trylock_mutex, OSP_Unlock_mutex
     *
     * \ingroup Atomics
     */

    osp_result_t OSP_Unlock_mutexes(MPI_Comm comm,
            int count,
            osp_mutex_t* *mutex_array);

    /* ********************************************************************** */
    /*                                                                        */
    /*               OSP external API - data transfer operations               */
    /*                                                                        */
    /* ********************************************************************** */

    /**
     * \brief Blocking copy of contiguous data from local memory to remote memory.
     *
     * The mnemonic is PUT BYTES from PTR to MEMREGION at RANK plus OFFSET.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.

     *
     * \see OSP_NbPut, OSP_MultiPut
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_Put(int                      bytes,
                         void*                    source_ptr,
                         osp_global_memregion_t*  memregion,
                         osp_locale_t             target_rank,
                         osp_offset_t             target_offset);

    /**
     * \brief Non-Blocking copy of contiguous data from local memory to remote memory.
     *
     * The mnemonic is NBPUT BYTES from PTR to MEMREGION at RANK plus OFFSET and get HANDLE back.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbPut, OSP_MultiPut
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbPut(int                      bytes,
                           void*                    source_ptr,
                           osp_global_memregion_t*  memregion,
                           osp_locale_t             target_rank,
                           osp_offset_t             target_offset,
                           osp_handle_t             handle);

    /**
     * \brief Blocking multiple-copy of contiguous data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     *
     * \see OSP_NbPut, OSP_MultiPut
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiPut(int*                     bytes,
                              void**                   source_ptr,
                              osp_global_memregion_t*  memregion,
                              osp_locale_t*            target_rank,
                              osp_offset_t*            target_offset);

    /**
     * \brief Non-Blocking multiple-copy of contiguous data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbPut, OSP_MultiPut
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiPut(int*                     bytes,
                                void**                   source_ptr,
                                osp_global_memregion_t*  memregion,
                                osp_locale_t*            target_rank,
                                osp_offset_t*            target_offset,
                                osp_handle_t             handle);

    /**
     * \brief Blocking copy of strided data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  stride_descr  The striding descriptor.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     *
     * \see OSP_Put, OSP_PutV, OSP_MultiPut, OSP_MultiPutS, OSP_MultiPutV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_PutS(osp_stride_descr_t        stride_descr,
                          void*                     source_ptr,
                          osp_global_memregion_t*   memregion,
                          osp_locale_t             target_rank,
                          osp_offset_t             target_offset);

    /**
     * \brief Non-Blocking copy of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Put, OSP_PutV, OSP_MultiPut, OSP_MultiPutS, OSP_MultiPutV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbPutS(osp_stride_descr_t        stride_descr,
                            void*                     source_ptr,
                            osp_global_memregion_t*   memregion,
                            osp_locale_t             target_rank,
                            osp_offset_t              target_offset,
                            osp_handle_t              handle);

    /**
     * \brief Blocking multiple-copy of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     *
     * \see OSP_Put, OSP_PutV, OSP_MultiPut, OSP_MultiPutS, OSP_MultiPutV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiPutS(osp_stride_descr_t*       stride_descr,
                                void**                    source_ptr,
                                osp_global_memregion_t*   memregion,
                                osp_locale_t*            target_rank,
                                osp_offset_t*             target_offset);

    /**
     * \brief Non-Blocking multiple-copy of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Put, OSP_PutV, OSP_MultiPut, OSP_MultiPutS, OSP_MultiPutV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiPutS(osp_stride_descr_t*       stride_descr,
                                void**                    source_ptr,
                                osp_global_memregion_t*   memregion,
                                osp_locale_t*            target_rank,
                                osp_offset_t*             target_offset,
                                osp_handle_t              handle);

    /**
     * \brief Blocking accumulate of contiguous data from local memory to remote memory.
     *
     * The mnemonic is ACC DESCR times BYTES from PTR to MEMREGION at RANK plus OFFSET.
     *
     * \param[out] rc            The error code.
     * \param[in]  acc_descr     The accumulate descriptor.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     *
     * \see OSP_NbPut, OSP_MultiPut
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_Acc(osp_acc_descr_t acc_descr,
            int bytes,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset);

    /**
     * \brief Non-Blocking accumulate of contiguous data from local memory to remote memory.
     *
     * The mnemonic is NBACC DESCR times BYTES from PTR to MEMREGION at RANK plus OFFSET
     * and get HANDLE back.
     *
     * \param[out] rc            The error code.
     * \param[in]  acc_descr     The accumulate descriptor.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbAcc, OSP_MultiAcc
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbAcc(osp_acc_descr_t acc_descr,
            int bytes,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking multiple-accumulate of contiguous data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  acc_descr     The accumulate descriptor.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     *
     * \see OSP_NbAcc, OSP_MultiAcc
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiAcc(osp_acc_descr_t* acc_descr,
            int* bytes,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset);

    /**
     * \brief Non-Blocking multiple-accumulate of contiguous data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  acc_descr     The accumulate descriptor.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbAcc, OSP_MultiAcc
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiAcc(osp_acc_descr_t* acc_descr,
            int* bytes,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking accumulate of strided data from local memory to remote memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  acc_descr     The accumulate descriptor.
     * \param[in]  stride_descr  The striding descriptor.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     *
     * \see OSP_Acc, OSP_MultiAcc, OSP_MultiAccS
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_AccS(osp_acc_descr_t acc_descr,
            osp_stride_descr_t stride_descr,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset);

    /**
     * \brief Non-Blocking accumulate of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  acc_descr       The accumulate descriptor.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Acc, OSP_MultiAcc, OSP_MultiAccS
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbAccS(osp_acc_descr_t acc_descr,
            osp_stride_descr_t stride_descr,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking multiple-accumulate of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  acc_descr       The accumulate descriptor.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     *
     * \see OSP_Acc, OSP_MultiAcc, OSP_MultiAccS
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiAccS(osp_acc_descr_t* acc_descr,
            osp_stride_descr_t* stride_descr,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset);

    /**
     * \brief Non-Blocking multiple-accumulate of strided data from local memory to remote memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  acc_descr       The accumulate descriptor.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Acc, OSP_MultiAcc, OSP_MultiAccS
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiAccS(osp_acc_descr_t* acc_descr,
            osp_stride_descr_t* stride_descr,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking copy of contiguous data from remote memory to local memory.
     *
     * The mnemonic is GET BYTES to PTR from MEMREGION at RANK plus OFFSET.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_Get(int bytes,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset);

    /**
     * \brief Non-Blocking copy of contiguous data from remote memory to local memory.
     *
     * The mnemonic is NBGET BYTES to PTR from MEMREGION at RANK plus OFFSET and get HANDLE back.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbGet(int bytes,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking multiple-copy of contiguous data from remote memory to local memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiGet(int* bytes,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset);

    /**
     * \brief Non-Blocking multiple-copy of contiguous data from remote memory to local memory.
     *
     * \param[out] rc            The error code.
     * \param[in]  bytes         Vector of amounts of data to transfer in bytes.
     * \param[in]  source_ptr    Vector of starting addresses in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Vector of ranks of the remote process.
     * \param[in]  target_offset Vector of offsets into the memregion at target_rank.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiGet(int* bytes,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking copy of strided data from remote memory to local memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     *
     * \see OSP_Get, OSP_GetV, OSP_MultiGet, OSP_MultiGetS, OSP_MultiGetV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_GetS(osp_stride_descr_t stride_descr,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset);

    /**
     * \brief Non-Blocking copy of strided data from remote memory to local memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Get, OSP_GetV, OSP_MultiGet, OSP_MultiGetS, OSP_MultiGetV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbGetS(osp_stride_descr_t stride_descr,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking multiple-copy of strided data from remote memory to local memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     *
     * \see OSP_Get, OSP_GetV, OSP_MultiGet, OSP_MultiGetS, OSP_MultiGetV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_MultiGetS(osp_stride_descr_t* stride_descr,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset);

    /**
     * \brief Non-Blocking multiple-copy of strided data from remote memory to local memory.
     *
     * \param[out] rc              The error code.
     * \param[in]  stride_descr    The striding descriptor.
     * \param[in]  source_ptr      Starting address in the (local) source memory.
     * \param[in]  memregion       Global memregion containing target memory.
     * \param[in]  target_rank     Rank of the remote process.
     * \param[in]  target_offset   Offset into the memregion at target_rank.
     * \param[out] handle          Opaque handle for the request.
     *
     * \see OSP_Get, OSP_GetV, OSP_MultiGet, OSP_MultiGetS, OSP_MultiGetV
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbMultiGetS(osp_stride_descr_t* stride_descr,
            void** source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t* target_rank,
            osp_offset_t* target_offset,
            osp_handle_t handle);

    /**
     * \brief Blocking get-and-accumulate operation (aka RMW) on arbitrary data.
     *        This operation is message-wise atomic.
     *
     * \warning Obviously, the user cannot mix RMW with other operations without
     *          proper synchronization.
     *
     * \brief Blocking read-modify-write of contiguous data from local memory to remote memory.
     *
     * The mnemonic is RMW BYTES from PTR1 to MEMREGION at RANK plus OFFSET
     * and return original at PTR2.
     *
     * \param[out] rc            The error code.
     * \param[in]  rmw_descr     The read-modify-write descriptor.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  in_ptr        Starting address in the (local) input memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     * \param[in]  out_ptr       Starting address in the (local) output memory.
     *
     * \see OSP_NbRmw
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_Rmw(osp_acc_descr_t acc_descr,
            int bytes,
            void* in_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            void* out_ptr);

    /**
     * \brief Non-Blocking read-modify-write of contiguous data from local memory to remote memory.
     *
     * The mnemonic is NBRMW BYTES from PTR1 to MEMREGION at RANK plus OFFSET,
     * return original at PTR2 and get HANDLE back.
     *
     * \param[out] rc            The error code.
     * \param[in]  rmw_descr     The read-modify-write descriptor.
     * \param[in]  bytes         Amount of data to transfer in bytes.
     * \param[in]  source_ptr    Starting address in the (local) source memory.
     * \param[in]  memregion     Global memregion containing target memory.
     * \param[in]  target_rank   Rank of the remote process.
     * \param[in]  target_offset Offset into the memregion at target_rank.
     * \param[in]  out_ptr       Starting address in the (local) output memory.
     * \param[out] handle        Opaque handle for the request.
     *
     * \see OSP_Rmw
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbRmw(osp_rmw_descr_t rmw_descr,
            int bytes,
            void* source_ptr,
            osp_global_memregion_t* memregion,
            osp_locale_t target_rank,
            osp_offset_t target_offset,
            void* out_ptr,
            osp_handle_t handle);

    /**
     * \brief Blocking copy of contiguous data from remote memory to remote memory.
     *
     * The mnemonic is COPY BYTES from MEMREGION at RANK plus OFFSET
     * to MEMREGION at RANK plus OFFSET.
     *
     * \param[out] rc             The error code.
     * \param[in]  bytes          Amount of data to transfer in bytes.
     * \param[in]  src_memregion  Global memregion containing target memory.
     * \param[in]  src_rank       Rank of the remote process.
     * \param[in]  src_offset     Offset into the memregion at target_rank.
     * \param[in]  dest_memregion Global memregion containing target memory.
     * \param[in]  dest_rank      Rank of the remote process.
     * \param[in]  dest_offset    Offset into the memregion at target_rank.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_Copy(int bytes,
            osp_global_memregion_t* src_memregion,
            osp_locale_t src_rank,
            osp_offset_t src_offset,
            osp_global_memregion_t* dest_memregion,
            osp_locale_t dest_rank,
            osp_offset_t dest_offset);

    /**
     * \brief Non-Blocking copy of contiguous data from remote memory to remote memory.
     *
     * The mnemonic is COPY BYTES from MEMREGION at RANK plus OFFSET
     * to MEMREGION at RANK plus OFFSET and get HANDLE back.
     *
     * \param[out] rc             The error code.
     * \param[in]  bytes          Amount of data to transfer in bytes.
     * \param[in]  src_memregion  Global memregion containing target memory.
     * \param[in]  src_rank       Rank of the remote process.
     * \param[in]  src_offset     Offset into the memregion at target_rank.
     * \param[in]  dest_memregion Global memregion containing target memory.
     * \param[in]  dest_rank      Rank of the remote process.
     * \param[in]  dest_offset    Offset into the memregion at target_rank.
     * \param[out] handle         Opaque handle for the request.
     *
     * \see OSP_NbGet, OSP_MultiGet
     *
     * \ingroup DATA_TRANSFER
     */

    osp_result_t OSP_NbCopy(int bytes,
            osp_global_memregion_t* src_memregion,
            osp_locale_t src_rank,
            osp_offset_t src_offset,
            osp_global_memregion_t* dest_memregion,
            osp_locale_t dest_rank,
            osp_offset_t dest_offset,
            osp_handle_t handle);

    /* ********************************************************************** */
    /*                                                                        */
    /*               OSP external API - synchronization and completion         */
    /*                                                                        */
    /* ********************************************************************** */

    /**
     * \brief On return, this call ensures that all outstanding put or accumulate
     *        operations to ranks within the communicator are complete remotely.
     *
     *        This operation is collective and blocking.  It has the effect of
     *        OSP_Flush_group called from every process in the group followed by OSP_Barrier_group.
     *
     * \param[out] rc            The error code.
     * \param[in] group          Group of processes to synchronize.
     *
     * \see OSP_NbSync_group, OSP_Flush_group.
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Sync_comm(MPI_Comm comm);

    /**
     * \brief When the handle state is OSP_TRUE, all outstanding put or accumulate
     *        operations are complete remotely within the entire group.
     *
     *        This operation is collective and blocking.  It has the effect of
     *        OSP_NbFlush_group called from every process in the group followed by OSP_NbBarrier_group.
     *
     * \param[out] rc            The error code.
     * \param[in] group          Group of processes to synchronize.
     *
     * \see OSP_Sync_group, OSP_NbFlush_group.
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_NbSync_comm(MPI_Comm comm, osp_handle_t handle);

    /**
     * \brief On return, this call ensures that all blocking and non-blocking put
     *        and accumulate operations sent to the target process have completed remotely.
     *
     *        This operation is local and blocking.
     *
     * \param[out] rc            The error code.
     * \param[in]  proc          Rank of the remote process.
     *
     * \see OSP_NbFlush, OSP_Flush_group, OSP_Flush_list
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_Flush_locale(osp_locale_t proc);

    /**
     * \brief When the handle state is OSP_TRUE, all blocking and non-blocking put
     *        and accumulate operations sent to the target process have completed remotely.
     *
     *        This operation is local and non-blocking.
     *
     * \param[out] rc            The error code.
     * \param[in]  proc          Rank of the remote process.
     *
     * \see OSP_Flush, OSP_NbFlush_group, OSP_NbFlush_list
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_NbFlush_locale(osp_locale_t proc, osp_handle_t handle);

    /**
     * \brief On return, this call ensures that all blocking and non-blocking put
     *        and accumulate operations have completed remotely at the processes given in the list.
     *
     *        This operation is local and blocking.
     *
     * \param[out] rc                The error code.
     * \param[in] count              Number of processes in vector.
     * \param[in] proc_list          Vector of processes to flush against.
     *
     * \see OSP_Flush, OSP_NbFlush_list
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_Flush_list(int count, osp_locale_t proc_list[]);

    /**
     * \brief When the handle state is OSP_TRUE, all blocking and non-blocking put
     *        and accumulate operations have completed remotely at the processes given in the list.
     *
     *        This operation is local and blocking.
     *
     * \param[out] rc                The error code.
     * \param[in] count              Number of processes in vector.
     * \param[in] proc_list          Vector of processes to flush against.
     *
     * \see OSP_NbFlush, OSP_Flush_list
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_NbFlush_list(int count, osp_locale_t proc_list[], osp_handle_t handle);

    /**
     * \brief On return, this call ensures that all blocking and non-blocking put
     *        and accumulate operations have completed remotely at the processes in the group.
     *
     *        This operation is local and blocking.
     *
     * \param[out] rc                The error code.
     * \param[in] count              Number of processes in vector.
     * \param[in] proc_list          List of processes to flush against.
     *
     * \see OSP_Flush, OSP_NbFlush_group
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_Flush_comm(MPI_Comm comm);

    /**
     * \brief When the handle state is OSP_TRUE, all blocking and non-blocking put
     *        and accumulate operations have completed remotely at the processes in the group.
     *
     *        This operation is local and blocking.
     *
     * \param[out] rc                The error code.
     * \param[in] count              Number of processes in vector.
     * \param[in] proc_list          List of processes to flush against.
     *
     * \see OSP_NbFlush, OSP_Flush_group
     *
     * \ingroup COMPLETION
     */

    osp_result_t OSP_NbFlush_group(MPI_Comm comm, osp_handle_t handle);

    /**
     * \brief Blocks on completion of all non-blocking handles.
     *
     * \param[out] rc                The error code.
     *
     * \see osp_handle_t, OSP_Wait_handle_list, OSP_Test_handle
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Wait_handle_all(void);

    /**
     * \brief Blocks on completion of a non-blocking handle.
     *
     * \param[in] handle      Non-blocking handle upon which to be waited.
     *
     * \see osp_handle_t, OSP_Wait_handle_list, OSP_Test_handle
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Wait_handle(osp_handle_t handle);

    /**
     * \brief Blocks on completion of all elements in a vector of non-blocking handles.
     *
     * \param[in] count        Number of handles in vector.
     * \param[in] handles      Vector of non-blocking handles upon which to be waited.
     *
     * \see osp_handle_t, OSP_Wait_handle, OSP_Test_handle_list
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Wait_handle_list(int count, osp_handle_t* handles);

    /**
     * \brief Blocks on completion of a non-blocking handle.
     *
     * \param[in]  handle        Non-blocking handle upon which to be waited.
     * \param[out] completed     Handle status.
     *
     * \see osp_handle_t, OSP_Wait_handle, OSP_Test_handle_list
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Test_handle(osp_handle_t handle,
            osp_bool_t* completed);

    /**
     * \brief Blocks on completion of all elements in a vector of non-blocking handles.
     *
     * \param[in]  count        Number of handles in vector.
     * \param[in]  handles      Vector of non-blocking handles upon which to be waited.
     * \param[out] completed    Vector of handle statuses.
     *
     * \see osp_handle_t, OSP_Test_handle, OSP_Wait_handle_list
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Test_handle_list(int count,
            osp_handle_t *handles,
            osp_bool_t* *completed);

    /**
     * \brief Resets a non-blocking handle.
     *
     * \fixme What is this even good for?
     *
     * \param[in] handle      Non-blocking handle to be reset.
     *
     * \see osp_handle_t, OSP_Reset_handle_list
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Reset_handle(osp_handle_t* handle);

    /**
     * \brief Resets a non-blocking handle.
     *
     * \fixme What is this even good for?
     *
     * \param[in]  count        Number of handles in vector.
     * \param[in] handle      Non-blocking handle to be reset.
     *
     * \see osp_handle_t, OSP_Reset_handle
     *
     * \ingroup SYNCHRONIZATION
     */

    osp_result_t OSP_Reset_handle_list(int count, osp_handle_t* handle);

    /*! @} */

    /* ********************************************************************** */
    /*                                                                        */
    /*               OSP version information                                   */
    /*                                                                        */
    /* ********************************************************************** */

    /* OSP_VERSION is the version string. OSP_NUMVERSION is the
     * numeric version that can be used in numeric comparisons.
     *
     * OSP_VERSION uses the following format:
     * Version: [MAJ].[MIN].[REV][EXT][EXT_NUMBER]
     * Example: 1.0.7rc1 has
     *          MAJ = 1
     *          MIN = 0
     *          REV = 7
     *          EXT = rc
     *          EXT_NUMBER = 1
     *
     * OSP_NUMVERSION will convert EXT to a format number:
     *          ALPHA (a) = 0
     *          BETA (b)  = 1
     *          RC (rc)   = 2
     *          PATCH (p) = 3
     * Regular releases are treated as patch 0
     *
     * Numeric version will have 1 digit for MAJ, 2 digits for MIN, 2
     * digits for REV, 1 digit for EXT and 2 digits for EXT_NUMBER. So,
     * 1.0.7rc1 will have the numeric version 10007201.
     */
#define OSP_VERSION "@OSP_VERSION@"
#define OSP_NUMVERSION @OSP_NUMVERSION@

#define OSP_RELEASE_TYPE_ALPHA  0
#define OSP_RELEASE_TYPE_BETA   1
#define OSP_RELEASE_TYPE_RC     2
#define OSP_RELEASE_TYPE_PATCH  3

#define OSP_CALC_VERSION(MAJOR, MINOR, REVISION, TYPE, PATCH) \
        (((MAJOR) * 10000000) + ((MINOR) * 100000) + ((REVISION) * 1000) + ((TYPE) * 100) + (PATCH))

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* OSP_H_INCLUDED */
