INCLUDE  = -I/bgsys/drivers/ppcfloor
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk
LIBS     = -L/bgsys/drivers/ppcfloor/spi/lib -lSPI -lSPI_cnk -lrt -lpthread

CC       = powerpc64-bgq-linux-gcc
CFLAGS   = -g -O2 -Wall -std=gnu99 $(INCLUDE)

LD       = $(CC)
LDFLAGS  = $(CFLAGS)

all: test_barrier.x test_lock.x test_counter.x test_pthread.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o 
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.x

realclean: clean

