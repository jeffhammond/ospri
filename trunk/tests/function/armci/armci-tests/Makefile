# Environment variable OSP_INSTALL should point to the install directory of OSP 

CC      = mpixlc_r 
COPT    = -g -O0

INCLUDE = -I. -I$(OSP_INSTALL)/include 
LIBS    = -L$(OSP_INSTALL)/lib -losp 

LD      = $(CC)
CFLAGS  = $(COPT) $(INCLUDE)
LDFLAGS = $(COPT)

TESTS := $(patsubst %.c,%.x,$(wildcard *.c))
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(TESTS)
	mv perf.x perf/.
	mv msgcheck.x msgcheck/.
	mv test-ibm.x test-ibm/.
	mv test2.x test2/.
	mv test.x test/.
	mv perf_nb.x perf_nb/.
	mv perf_aggr.x perf_aggr/.

%.x: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

