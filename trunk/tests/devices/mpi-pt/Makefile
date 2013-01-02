CC      = mpicc
#CC      = ~/eclipse/MPICH2/mpich2-trunk/install-clang/bin/mpicc
COPT    = -g -O0 -Wall -std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT)
LDFLAGS = $(COPT) safemalloc.o
LIBS    = -lm

TESTS := basic.x bug.x

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
	$(RM) $(RMFLAGS) -r *.dSYM

