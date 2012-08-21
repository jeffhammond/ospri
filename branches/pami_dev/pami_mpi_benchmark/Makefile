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

all: pami mpi
mpi: mpi_get.x mpi_put.x mpi_send_recv_pair.x mpi_gather.x mpi_allgather.x mpi_send_recv_topo.x
pami: pami_send_pingpong_perf.x pami_send_alltoone.x pami_send_alltoall.x #pami_rget_perf.x pami_rput_perf.x

%.x: %.o q5d.o
#%.x: %.o 
	$(LD) $(LDFLAGS) $< q5d.o -o $@

%.o: %.c q5d.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x core.*

