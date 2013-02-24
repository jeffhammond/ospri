ifndef COMPILER
  COMPILER = GNU
endif

ifeq	($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq	($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -qstrict --qlanglvl=extc99 -O3 -qarch=qp -qtune=qp
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

all: safemalloc.o test-memory.x test-barrier.x test-bcast.x test-allreduce.x test-allgather.c

%.o: %.c safemalloc.h preamble.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o safemalloc.o Makefile
	$(LD) $(LDFLAGS) $< safemalloc.o -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x core.*

