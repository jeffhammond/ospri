CC      = mpicc
COPT    = -g -O0 -Wall #-std=c99

LD      = $(CC)
CFLAGS  = $(COPT)
LDFLAGS = $(COPT)
LIBS    = -lrt

TESTS   := $(patsubst %.c,%.x,$(wildcard test*.c))
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TESTS)

%.x: %.o ospu_mpi_comm_node.o
	$(LD) $(LDFLAGS) $< ospu_mpi_comm_node.o $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

