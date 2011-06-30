/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  Copyright (C) 2010 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Kazutomo Yoshii authored the original version of this code.
 *
 */

#if defined(__GNUC__)

    #if defined(__i386__)

        unsigned long long OSPU_Time_cycles(void)
        {
          unsigned long long int x;
             __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
             return x;
        }

        #elif defined(__x86_64__)

        unsigned long long OSPU_Time_cycles(void)
        {
          unsigned hi, lo;
          __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
          return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
        }

        #elif defined(__powerpc__)

        unsigned long long OSPU_Time_cycles(void)
        {
          unsigned long long int result=0;
          unsigned long int upper, lower,tmp;
          __asm__ volatile(
                        "0:                  \n"
                        "\tmftbu   %0           \n"
                        "\tmftb    %1           \n"
                        "\tmftbu   %2           \n"
                        "\tcmpw    %2,%0        \n"
                        "\tbne     0b         \n"
                        : "=r"(upper),"=r"(lower),"=r"(tmp)
                        );
          result = upper;
          result = result<<32;
          result = result|lower;

          return(result);
        }

    #else

        #error "No tick counter is available!"

    #endif

#else

    #error "Requires GNU-style ASM!"

#endif