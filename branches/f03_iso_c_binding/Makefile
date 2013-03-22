CC		= gcc
CFLAGS  = -g -Wall

FC      = xlf2003_r
FFLAGS  = -g -qintsize=8

LD	    = $(FC)
LDFLAGS = $(FFLAGS)

LIBS    = -lm

all: size_t.x c_size_t.x basic.x

%.x: %.f03
	$(FC) $(FFLAGS) $< $(LIBS) -o $@

%.x: %.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
