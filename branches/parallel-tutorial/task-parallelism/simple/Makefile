CC       = mpicc
CFLAGS   = -g -O2 -DPARALLEL
LD       = $(CC)
LDFLAGS  = $(CFLAGS)
LIBS     = -lm

all: serial.x parallel.x

OBJECTS = init.o foo.o bar.o 

%.x: %.o $(OBJECTS)
	$(LD) $(LDFLAGS) $< $(OBJECTS) $(LIBS) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o

realclean: clean
	-rm -f *.x

distclean: realclean

