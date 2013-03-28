INCLUDE  = -I/bgsys/drivers/ppcfloor
INCLUDE += -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk
LIBS     = -L/bgsys/drivers/ppcfloor/spi/lib -lSPI -lSPI_cnk -lrt -lpthread

CC       = powerpc64-bgq-linux-gcc
CFLAGS   = -g -O2 -Wall -std=gnu99 $(INCLUDE)

LD       = $(CC)
LDFLAGS  = $(CFLAGS)

all: test_lock.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o 
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.x

