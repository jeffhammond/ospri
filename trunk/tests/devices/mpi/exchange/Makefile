#gcc gcc.legacy xl xl.legacy xl.legacy.ndebug xl.ndebug

#MPICH_CC=powerpc64-bgq-linux-gcc

ifeq ($(MPI_FLAVOR),)
all:
	@echo "You must set MPI_FLAVOR to one of these: gcc gcc.legacy xl xl.legacy xl.legacy.ndebug xl.ndebug"
else
all: tests
endif

CC      = /bgsys/drivers/ppcfloor/comm/$(MPI_FLAVOR)/bin/mpicc
ifeq ($(MPICH_CC),)
  # DEFAULT COMPILER
  ifeq ($(MPI_FLAVOR),gcc)
	COPT    = -g -O2 -Wall -std=c99
  endif
  ifeq ($(MPI_FLAVOR),gcc.legacy)
	COPT    = -g -O2 -Wall -std=c99
  endif
  ifeq ($(MPI_FLAVOR),xl)
	COPT    = -qstrict -g -O3 -qlanglvl=stdc99
  endif
  ifeq ($(MPI_FLAVOR),xl.legacy)
	COPT    = -qstrict -g -O3 -qlanglvl=stdc99
  endif
  ifeq ($(MPI_FLAVOR),xl.legacy.ndebug)
	COPT    = -qstrict -O3 -qlanglvl=stdc99
  endif
  ifeq ($(MPI_FLAVOR),xl.ndebug)
	COPT    = -qstrict -O3 -qlanglvl=stdc99
  endif
else
  # OVERRIDE FLAGS WHEN WE MESS WITH THINGS
  ifeq ($(MPICH_CC),powerpc64-bgq-linux-gcc)
	COPT    = -g -O2 -Wall -std=c99
  endif
  ifeq ($(MPICH_CC),bbgxlc_r)
	COPT    = -qstrict -g -O3 -qlanglvl=stdc99
  endif
endif


LD      = $(CC)
CFLAGS  = $(COPT) -I.
LDFLAGS = $(COPT) -lm

TESTS   = test_torus_exchange.$(MPI_FLAVOR).x
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
LIBS    = safemalloc.o

tests: $(TESTS)

%.$(MPI_FLAVOR).x: %.o safemalloc.o 
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c safemalloc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

