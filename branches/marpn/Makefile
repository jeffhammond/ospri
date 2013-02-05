CC      = mpicc
COPT    = -g -O2 -Wall #-std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT) -DCHECK_ERRORS

LDFLAGS = $(COPT)
LIBS    = libmpiarbrpn.a

all: hello.x

%.x: %.o libmpiarbrpn.a
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o

realclean: clean
	-rm -f *.x
