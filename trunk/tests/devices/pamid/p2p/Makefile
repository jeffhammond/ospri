ifndef COMPILER
  COMPILER = GNU
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = /bgsys/drivers/ppcfloor/comm/xl.legacy.ndebug/bin/mpixlc_r 
  COPT    = -g -qlanglvl=extc99 -O5 -qarch=qp -qtune=qp
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

ifdef PROGRESS
    CFLAGS += -DPROGRESS_THREAD
endif

obj = safemalloc.o barrier.o allgather.o

all: mpi pami

pami: send.x endpoint.x put.x rput.x get.x rget.x rmw.x type1.x
mpi: mpi-put.x mpi-rput.x mpi-rput-ibm.x mpi-get.x mpi-rget.x
bugs: bug-allgather.x

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

