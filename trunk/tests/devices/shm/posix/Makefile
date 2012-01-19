CC      = mpicc
COPT    = -g -O0 -Wall #-std=c99

LD      = $(CC)
CFLAGS  = $(COPT) -I/bgsys/drivers/ppcfloor/arch/include/
LDFLAGS = $(COPT)
LIBS    = -lrt

TESTS   := $(patsubst %.c,%.x,$(wildcard *.c))
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TESTS)

%.x: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)
	$(RM) $(RMFLAGS) core.*

