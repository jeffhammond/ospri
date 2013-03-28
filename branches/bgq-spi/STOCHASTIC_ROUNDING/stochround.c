#ifndef NOHELP
#error mpicc -g -O2 -Wall -std=gnu99 -I/bgsys/drivers/ppcfloor -DNOHELP stochround.c -o stochround.x
#endif

#include <stdio.h>
#include <stdlib.h>
#include <hwi/include/bqc/A2_core.h> 
#include <spi/include/kernel/process.h>
#include <spi/include/kernel/location.h>

#if DOCS
/*!
 * \brief Return the value of the AXUCR0 register.
 *
 * If the caller's process owns the whole core (that is, if the node is
 * configured with at most 16 processes), access to the stochastic rounding
 * control bits and to all the thread signalling control bits is provided.
 * Otherwise access is restricted to just the calling thread's signalling
 * control bits.
 * 
 * \return AXUCR0 value (masked to just those bits the caller is allowed to access)
 */
__INLINE__
uint64_t Kernel_GetAXUCR0(void);

/*!
 * \brief Set the value of the AXUCR0 register.
 *
 * If the caller's process owns the whole core (that is, if the node is
 * configured with at most 16 processes), changes to the stochastic rounding
 * control bits and to all the thread signalling control bits are allowed.
 * Otherwise changes are restricted to just the calling thread's signalling
 * control bits.
 *
 * \param[in]  value   The value to be set into AXUCR0.
 * 
 * \return Error indication
 * \retval  0 success
 * \retval  EINVAL if disallowed bits are included in value
 * \retval  ENOSYS if not supported
 *
 */

__INLINE__
uint64_t Kernel_SetAXUCR0(uint64_t value);
#endif

int main(int argc, char* argv[])
{
  if (0==Kernel_GetRank())
  {
    uint64_t a = Kernel_GetAXUCR0();
    printf("%lu = Kernel_GetAXUCR0() \n", (uint64_t) a);
    for (int i=0;i<64;i++)
    {
        printf("%s", (a%2)==0 ? "0" : "1");
        a /= 2;
    }
    printf("\n");

    uint64_t b = argc>1 ? atol(argv[1]) : 0;

    uint64_t c = Kernel_SetAXUCR0(b);
    printf("%lu = Kernel_SetAXUCR0(%lu) \n", (uint64_t) c, (uint64_t) b);
    for (int i=0;i<64;i++)
    {
        printf("%s", (b%2)==0 ? "0" : "1");
        b /= 2;
    }
    printf("\n");

    if      (c==0)      printf("SUCCESS \n");
    else if (c==EINVAL) printf("EINVAL  \n");
    else if (c==ENOSYS) printf("ENOSYS  \n");
  }
  return 0;
}
