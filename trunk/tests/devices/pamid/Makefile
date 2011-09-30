ifndef COMPILER
  COMPILER = GNU
endif

ifeq	($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -O2 -Wall
  LABEL   = gnu
endif

ifeq	($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -O3 -qstrict
  LABEL	= xl
endif

LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lpthread

all: hello.x query.x barrier.x allgather.x allreduce.x rget.x

%.x: %.o
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x.xl *.x.gnu core.*

