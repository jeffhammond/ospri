
CC=/bgsys/drivers/ppcfloor/comm/gcc/bin/mpicc
#CC=/bghome/blocksom/development/issues/v1r1m2/bgq/work/comm/gcc/bin/mpicc
CFLAGS=-g
LDLIBS=-lm


OBJECTS=main.o simple_query.o simple_barrier.o simple_async_progress.o accumulate_data_functions.o

TESTS=test.cnk

.PHONEY: all clean
all: $(TESTS)

clean:
	$(RM) *.o *.cnk

%.cnk: %.o $(OBJECTS)
	$(CC) $(LDFLAGS) $< $(OBJECTS) -o $@ $(LDLIBS) 

#	$(CC) $(CFLAGS) $(CPPFLAGS) $< $(OBJECTS) -o $@

#%.cnk: %.o $(TEST_OBJECTS)
#	$(LD) $(CFLAGS) $(CPPFLAGS) $< $(TEST_OBJECTS) -o $@





