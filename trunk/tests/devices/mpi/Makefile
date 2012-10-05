CC      = mpicc
COPT    = -g -O2 -Wall -std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT)
# this is a dirty hack to detect BGP
ifeq ($(PLATFORM),linux-sles10-ppc64)
CFLAGS  += -I/bgsys/drivers/ppcfloor/arch/include/
endif
LDFLAGS = $(COPT) safemalloc.o
LIBS    = -lm

TESTS := allreduce2.x bug.x hello_mpi.x passive-target-get.x reduce_scatter_bug.x rma_pthreads2.x \
		 test_bgp_3d_exchange.x allreduce.x coll.x mpi_pthreads.x passive-target-put.x \
		 rma_pthreads.x test_bw.x bcast.x get_proc_name.x passive-target-acc.x \
		 pt-rma.x test_bgp_3d_exchange_simple.x test_torus_exchange.x james_allreduce.x reduce_scatter.x

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TESTS) safemalloc.o

%.x: %.o safemalloc.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

