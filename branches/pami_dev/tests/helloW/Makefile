#ifndef COMPILER
COMPILER =XL
#endif

ifeq    ($(COMPILER),GNU)
 CC      = mpicc
 COPT    = -g -O2 -Wall -fopenmp
 LABEL   = gnu
endif

ifeq    ($(COMPILER),XL)
 CC      = mpixlc_r
 COPT    = -g -O3 -qstrict -qsmp=omp:noauto
 LABEL  = xl
endif

CC = mpixlc_r

LD      = $(CC)
CFLAGS  = $(COPT)
#CFLAGS  = $(COPT) -I/bgsys/drivers/ppcfloor/comm/sys/include
LDFLAGS = $(COPT) -lm -lpthread -lpami

all: testhello.x

%.x: %.o
    $(LD) $(LDFLAGS) $< -o $@.$(LABEL)

%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    $(RM) $(RMFLAGS) *.o
    $(RM) $(RMFLAGS) *.lst

realclean: clean
    $(RM) $(RMFLAGS) *.x.xl *.x.gnu core.*
