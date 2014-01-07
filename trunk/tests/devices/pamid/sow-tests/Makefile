ifndef COMPILER
  COMPILER = GNU
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -std=gnu99 -O2 -mcpu=a2 -mtune=a2 
  #COPT   += -fopenmp
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -qstrict -qlanglvl=extc99 -O3 -qarch=qp -qtune=qp
  COPT   += -qsmp=omp:noauto
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

CFLAGS += -DPROGRESS_THREAD -DDEBUG

obj = safemalloc.o barrier.o allgather.o alltoall.o

all: pami mpi coll

pami: put.x rput.x get.x rget.x send.x acc1.x rmw.x
coll: rput-a2a.x rput-a2a-mt.x rget-a2a-mt.x
mpi: rsend.x

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

