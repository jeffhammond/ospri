ifndef COMPILER
  COMPILER = GNU
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -qstrict --qlanglvl=extc99 -O3 -qarch=qp -qtune=qp
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

ifdef PROGRESS
    CFLAGS += -DPROGRESS_THREAD
endif

all: endpoint.x put.x mpi-put.x mpi-rput.x mpi-rput-ibm.x #mpi-get.x mpi-rget.x rget.x new-rget.x
obj = safemalloc.o barrier.o allgather.o

%.o: %.c safemalloc.h coll.h
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(obj)
	$(LD) $(LDFLAGS) $< $(obj) -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

