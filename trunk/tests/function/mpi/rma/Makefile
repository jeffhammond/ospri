CC      = mpicc
COPT    = -g -O2 #-Wall -std=gnu99

LD      = $(CC)
CFLAGS  = $(COPT)

# this is a dirty hack to detect BGP
ifeq ($(PLATFORM),linux-sles10-ppc64)
CFLAGS  += -DBGP -I/bgsys/drivers/ppcfloor/arch/include/
endif
# this is a dirty hack to detect BGQ
ifeq ($(PLATFORM),linux-rhel_6-ppc64)
CFLAGS  += -DBGQ
endif

LDFLAGS = $(COPT)
LIBS    = -lm

TESTS := driver.x

OBJECTS := safemalloc.o driver.o

all: $(TESTS)

%.x: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

%.o: %.c mympix.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)

