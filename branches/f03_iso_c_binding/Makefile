FC      = gfortran
FFLAGS  = -g -Wall

LD	    = gfortran
LDFLAGS = -g -Wall

all: c_size_t.x

%.o: %.f03
	$(FC) $(FFLAGS) -c $< -o $@

%.x: %.o
	$(FC) $(FFLAGS) $< -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
