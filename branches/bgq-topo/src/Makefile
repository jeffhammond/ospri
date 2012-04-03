#INCLUDE  = -I/bgsys/drivers/ppcfloor 
#INCLUDE += -I/bgsys/drivers/ppcfloor/firmware/include
#INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel 
#INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk
#CC      = powerpc64-bgq-linux-gcc

CC        = mpicc
CFLAGS    = -g -O0 $(INCLUDE)

CXX       = mpicxx
CXXFLAGS  = -g -O0 $(INCLUDE)

LD        = mpicxx
LDFLAGS   = -g -O0

all: spi.x mpix.x torus.x

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(LDFLAGS) $< -o $@

clean:
	-rm -f *.o

distclean:
	-rm -f *.x

