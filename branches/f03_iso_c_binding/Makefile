ifndef COMPILER
COMPILER=GCC
endif

ifeq ($(COMPILER),CRAY)
CC	     = craycc
CFLAGS  = -g 
FC      = crayftn
FFLAGS  = -g -s integer64
endif

ifeq ($(COMPILER),INTEL)
CC	    = icc
CFLAGS  = -g 
FC      = ifort
FFLAGS  = -g -i8 -free -stand f03
endif

ifeq ($(COMPILER),XL)
CC      = xlc_r
CFLAGS  = -g
FC      = xlf2003_r
FFLAGS  = -g -qintsize=8
endif

ifeq ($(COMPILER),GCC)
CC      = gcc
CFLAGS  = -g -Wall
FC      = gfortran
FFLAGS  = -g -fdefault-integer-8
endif

LD	    = $(FC)
LDFLAGS = $(FFLAGS)

LIBS    = -lm

all: size_t.x c_size_t.x basic.x

%.x: %.f
	$(FC) $(FFLAGS) $< $(LIBS) -o $@

%.x: %.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
