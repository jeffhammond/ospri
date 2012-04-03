/* written by Paul Hargrove */

#include <stdint.h>
#include <stdio.h>

// Requires -I/bgsys/drivers/ppcfloor
#include "cnk/include/SPI_syscalls.h"
#include "firmware/include/personality.h"

int main(int argc, char **argv) {
 Personality_t pers;
 int rc = CNK_SPI_SYSCALL_2(GET_PERSONALITY, (uintptr_t)&pers, (uint64_t)sizeof(pers));

 if (rc == 0) {
   int _blkTorus[5];
   for (int i = 0; i < 5; ++i) {
     _blkTorus[i] = ND_GET_TORUS(i,pers.Network_Config.NetFlags);
   }

   printf(" %d %d %d %d %d\n", _blkTorus[0], _blkTorus[1], _blkTorus[2], _blkTorus[3], _blkTorus[4]);
 } else {
   printf("syscall failed!\n");
 }
 fflush(stdout);

 return 0;
}
