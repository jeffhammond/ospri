CC      = mpicc
COPT    = -g -O2 -Wall -std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT)

# this is a dirty hack to detect BGP
ifeq ($(PLATFORM),linux-sles10-ppc64)
CFLAGS  += -DBGP -I/bgsys/drivers/ppcfloor/arch/include/
endif
# this is a dirty hack to detect BGQ
ifeq ($(PLATFORM),linux-rhel_6-ppc64)
CFLAGS  += -DBGQ
endif

LDFLAGS = $(COPT)
LIBS    = -lm

TESTS := driver.x bcast-zero.x dtype-zero-vector.x comm_split.x

OBJECTS := allgather.o alltoall.o bcast.o driver.o reducescatter.o safemalloc.o

HEADERS := safemalloc.h driver.h

all: $(TESTS)

%.x: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

bcast-zero.x: bcast-zero.c
	$(CC) $(CFLAGS) $< -o $@

dtype-zero-vector.x: dtype-zero-vector.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

