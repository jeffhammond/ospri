CC      = mpicc
COPT    = -g -O2 -Wall -std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT) 
#CFLAGS += -DDO_COMM_WORLD -DDO_COMM_WORLD_JITTER -DDO_COMM_WORLD_DUP -DDO_WORLD_REORDERED 
CFLAGS += -DDO_COMM_WORLD -DDO_COMM_WORLD_DUP -DDO_WORLD_REORDERED 
CFLAGS += -DDO_LEFT_RIGHT -DDO_ODD_EVEN -DDO_WORLD_MINUS_ONE -DDO_GEOM_PROG
#CFLAGS += -DDEBUG

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

OBJECTS := driver.o safemalloc.o bcast.o reduce.o allreduce.o gather.o allgather.o scatter.o alltoall.o reducescatter.o

HEADERS := safemalloc.h driver.h

all: $(TESTS)

%.x: %.c $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

bcast-zero.x: bcast-zero.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

dtype-zero-vector.x: dtype-zero-vector.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

comm_split.x: comm_split.c safemalloc.h safemalloc.o
	$(CC) $(CFLAGS) $< safemalloc.o -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

fun:
	say Open M P I sucks


