ifndef COMPILER
  COMPILER = GNU
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -std=gnu99 -O2 -mcpu=a2 -mtune=a2 
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -qstrict --qlanglvl=extc99 -O3 -qarch=qp -qtune=qp
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

CFLAGS += -DDEBUG

obj = safemalloc.o barrier.o

all: send.x acc1.x mpi.x

%.o: %.c safemalloc.h coll.h preamble.h
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(obj)
	$(LD) $(LDFLAGS) $< $(obj) -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

