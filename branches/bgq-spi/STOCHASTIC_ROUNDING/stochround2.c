#ifndef NOHELP
#error mpicc -g -O2 -Wall -I/bgsys/drivers/ppcfloor -DNOHELP stochround2.c -o stochround2.x
#endif

#include <stdio.h> 

#include <hwi/include/bqc/A2_core.h> 
#include <spi/include/kernel/process.h> 

int main(int argc, char **argv) 
{ 
    uint64_t value; 

    value = Kernel_GetAXUCR0(); 
    printf("AXUCR0 = %lx\n", value); 

    uint64_t rc; 
    rc = Kernel_SetAXUCR0(AXUCR0_SR_ENABLE | AXUCR0_LFSR_RESET); 
    if (rc != 0) { 
        printf("SetAXUCR0 failed, rc = %ld.\n", rc); 
        return 1; 
    } 
    rc = Kernel_SetAXUCR0(AXUCR0_SR_ENABLE); 
    if (rc != 0) { 
        printf("SetAXUCR0 failed, rc = %ld.\n", rc); 
        return 1; 
    } 

    value = Kernel_GetAXUCR0(); 
    printf("AXUCR0 = %lx\n", value); 

    return 0; 
} 
