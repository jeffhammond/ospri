CC      = mpicc
COPT    = -g -O2 -Wall -std=c99

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lm

TESTS := $(patsubst %.c,%.x,$(wildcard test*.c))
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
LIBS := safemalloc.o

all: $(TESTS)

%.x: %.o safemalloc.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c safemalloc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

