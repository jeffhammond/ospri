INCLUDE  = -I/bgsys/drivers/ppcfloor 
INCLUDE += -I/bgsys/drivers/ppcfloor/firmware/include
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel 
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk

CC        = mpicc
#CC        = powerpc64-bgq-linux-gcc
CFLAGS    = -g -O2 -Wall -std=gnu99 $(INCLUDE)

LD        = $(CC)
#LD        = $(CC)
LDFLAGS   = -g -O2 -Wall -std=gnu99

AR        = powerpc64-bgq-linux-ar

all: libq5d.a

check: capi.x

libq5d.a: q5d.o
	$(AR) -r libq5d.a q5d.o

q5d.o: q5d.c q5d.h
	$(CC) $(CFLAGS) -c q5d.c -o q5d.o

capi.x: capi.o libq5d.a
	$(LD) capi.o -L. -lq5d -o capi.x

capi.o: capi.c q5d.h
	$(CC) $(CFLAGS) -c capi.c -o capi.o

clean:
	-rm -f *.o

distclean: clean
	-rm -f *.a
	-rm -f *.x

