ifndef COMPILER
 COMPILER=GCC
endif

ifeq ($(COMPILER),CRAY)
 CC	      = craycc
 CFLAGS   = -g 
 FC       = crayftn
 FFLAGS   = -g -f free 
 ifeq ($(INTSIZE),8)
  FFLAGS +=-s integer64
 endif
endif

ifeq ($(COMPILER),INTEL)
 CC	      = icc
 CFLAGS   = -g 
 FC       = ifort
 FFLAGS   = -g -free -stand f03
 ifeq ($(INTSIZE),8)
  FFLAGS += -i8
 endif
endif

ifeq ($(COMPILER),XL)
 CC       = xlc_r
 CFLAGS   = -g
 FC       = xlf2003_r
 FFLAGS   = -g
 ifeq ($(INTSIZE),8)
  FFLAGS += -qintsize=8
 endif
endif

ifeq ($(COMPILER),GCC)
 CC       = gcc
 CFLAGS   = -g -Wall
 FC       = gfortran
 FFLAGS   = -g
 ifeq ($(INTSIZE),8)
  FFLAGS += -fdefault-integer-8
 endif
endif

LD	    = $(FC)
LDFLAGS = $(FFLAGS)

LIBS    = -lm

all: size_t.x c_size_t.x #basic.x

%.x: %.f
	$(FC) $(FFLAGS) $< $(LIBS) -o $@

%.x: %.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
