INCLUDE  = -I/bgsys/drivers/ppcfloor 
INCLUDE += -I/bgsys/drivers/ppcfloor/firmware/include
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel 
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk

CC        = powerpc64-bgq-linux-gcc
CFLAGS    = -g -O2 -Wall -std=gnu99 $(INCLUDE)

LD        = powerpc64-bgq-linux-gcc
LDFLAGS   = -g -O2 -Wall -std=gnu99

all: libbgqtopo.a

libbgqtopo.a: impl.o q5d.o

impl.o: impl.c impl.h
	$(CC) $(CFLAGS) -c impl.c -o impl.o

q5d.o: q5d.c q5d.h impl.h
	$(CC) $(CFLAGS) -c q5d.c -o q5d.o

clean:
	-rm -f *.o

distclean: clean
	-rm -f *.a

