FC      = gfortran
FFLAGS  = -g 

LD	    = $(FC)
LDFLAGS = $(FFLAGS)

all: c_size_t.x basic.x

%.o: %.f03
	$(FC) $(FFLAGS) -c $< -o $@

%.x: %.o
	$(FC) $(FFLAGS) $< -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
