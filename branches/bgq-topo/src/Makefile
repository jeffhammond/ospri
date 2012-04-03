INCLUDE  = -I/bgsys/drivers/ppcfloor 
INCLUDE += -I/bgsys/drivers/ppcfloor/firmware/include
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel 
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk

CC        = powerpc64-bgq-linux-gcc
CFLAGS    = -g -O2 -Wall -std=gnu99 $(INCLUDE)

LD        = powerpc64-bgq-linux-gcc
LDFLAGS   = -g -O2 -Wall -std=gnu99

all: libbgqtopo.a

libbgqtopo.a: torus.o core.o fortran.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o

distclean: clean
	-rm -f *.a

