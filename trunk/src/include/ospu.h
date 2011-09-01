/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#if !defined OSPU_H_INCLUDED
#define OSPU_H_INCLUDED

/* redundant with src/devices/common/compilers.h
 * some reorg of this stuff needed eventually    */
#define likely_if(x) if(__builtin_expect(x,1))
#define unlikely_if(x) if(__builtin_expect(x,0))

#include "ospconf.h"

#if defined HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#if defined HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if defined HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */

#if defined HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if defined HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#if defined HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if defined HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */

#if defined HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#if defined HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#if defined HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#if defined HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

#if defined HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#if !defined HAVE_MACRO_VA_ARGS
#error "VA_ARGS support is required"
#endif /* HAVE_MACRO_VA_ARGS */

/* FIXME: FUNC_ENTER/EXIT can be used for profiling in the future */

#define OSPU_FUNC_ENTER(...)
#define OSPU_FUNC_EXIT(...)

#if defined HAVE__FUNC__
#define OSPU_FUNC __func__
#elif defined HAVE_CAP__FUNC__
#define OSPU_FUNC __FUNC__
#elif defined HAVE__FUNCTION__
#define OSPU_FUNC __FUNCTION__
#endif

#if defined __FILE__ && defined OSPU_FUNC
    #define OSPU_error_printf(...)                                          \
        {                                                                   \
            fprintf(stderr, "%s (%s:%d): ", OSPU_FUNC, __FILE__, __LINE__);  \
            fprintf(stderr, __VA_ARGS__);                                   \
            fflush(stderr);                                                 \
        }
#elif defined __FILE__
    #define OSPU_error_printf(...)                               \
        {                                                        \
            fprintf(stderr, "%s (%d): ", __FILE__, __LINE__);    \
            fprintf(stderr, __VA_ARGS__);                        \
            fflush(stderr);                                      \
        }
#else
    #define OSPU_error_printf(...)                                          \
        {                                                                   \
            fprintf(stderr, __VA_ARGS__);                                   \
            fflush(stderr);                                                 \
        }
#endif

#define OSPU_output_printf(...)                                         \
    {                                                                   \
        fprintf(stdout, __VA_ARGS__);                                   \
        fflush(stdout);                                                 \
    }

#define OSPU_ASSERT_ABORT(x, ...)                                        \
    {                                                                   \
        unlikely_if (!(x)) {                                                     \
            OSPU_error_printf(__VA_ARGS__);                              \
            assert(0);                                                  \
        }                                                               \
    }

#define OSPU_ASSERT(x, status)                                           \
    {                                                                   \
        unlikely_if (!(x)) {                                                     \
            OSPU_ERR_SETANDJUMP(status, OSP_ERROR,                        \
                               "assert (%s) failed\n", #x);             \
        }                                                               \
    }

#define OSPU_WARNING(status, ...)                                      \
    {                                                                   \
        unlikely_if (status) {                                                   \
            OSPU_error_printf(__VA_ARGS__);                              \
        }                                                               \
    }

#define OSPU_ERR_ABORT(status, ...)                                      \
    {                                                                   \
        unlikely_if (status) {                                                   \
            OSPU_error_printf(__VA_ARGS__);                              \
            assert(0);                                                  \
        }                                                               \
    }

#define OSPU_ERR_POP(status, ...)                                        \
    {                                                                   \
        unlikely_if (status) {                                                   \
            OSPU_error_printf(__VA_ARGS__);                              \
            goto fn_fail;                                               \
        }                                                               \
    }

#define OSPU_ERR_SETANDJUMP(status, error, ...)                          \
    {                                                                   \
        status = error;                                                 \
        OSPU_ERR_POP(status, __VA_ARGS__);                               \
    }

#define OSPU_ERR_CHKANDJUMP(status, chk, error, ...)                     \
    {                                                                   \
        unlikely_if ((chk))                                                      \
            OSPU_ERR_SETANDJUMP(status, error, __VA_ARGS__);             \
    }

#define OSPU_ERR_POPANDSTMT(status, stmt, ... )                          \
    {                                                                   \
        unlikely_if (status) {                                                   \
            OSPU_error_printf(__VA_ARGS__);                              \
            stmt;                                                       \
        }                                                               \
    }

#ifdef OSP_DEBUG
#define OSPU_DEBUG_PRINT(args...)                                  \
do {                                                              \
    int __my_rank;                                                \
    __my_rank = MPI_Comm_rank(OSP_COMM_WORLD);                  \
    fprintf(stderr, "Debug Message from [%d] :", __my_rank);      \
    fprintf(stderr, args);                                        \
    fflush(stderr);                                               \
} while (0)
#else
#define OSPU_DEBUG_PRINT(args...)
#endif

#define OSPU_Malloc(ptr, num) posix_memalign(ptr, OSPC_ALIGNMENT, num)
#define OSPU_Free(ptr) free(ptr)
#define OSPU_Memset(ptr, val, num)  memset(ptr, val, num)
#define OSPU_Memcpy(trg, src, num)  memcpy(trg, src, num)

int OSPU_Put_memcpy(void* src,
                   void* dst,
                   int bytes);

int OSPU_PutS_memcpy(int stride_level,
                    int *block_sizes,
                    void* source_ptr,
                    int *src_stride_ar,
                    void* target_ptr,
                    int *trg_stride_ar);

int OSPU_Get_memcpy(void* src,
                   void* dst,
                   int bytes);

int OSPU_GetS_memcpy(int stride_level,
                    int *block_sizes,
                    void* source_ptr,
                    int *src_stride_ar,
                    void* target_ptr,
                    int *trg_stride_ar);

int OSPU_Acc_memcpy(void* source_ptr,
                   void* target_ptr,
                   int bytes,
                   osp_datatype_t osp_type,
                   void* scaling);

int OSPU_AccS_memcpy(int stride_level,
                    int *block_sizes,
                    void* source_ptr,
                    int *src_stride_ar,
                    void* target_ptr,
                    int *trg_stride_ar,
                    osp_datatype_t osp_type,
                    void* scaling);

#endif /* OSPU_H_INCLUDED */
