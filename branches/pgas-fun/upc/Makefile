UPC      = cc -h upc
UPCFLAGS = -g -O2 -DRELAXED
LD       = $(UPC)
LDFLAGS  = $(UPCFLAGS)
LIBS     = -lhugetlbfs

all: hello.x barrier.x shared.x

%.x: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.upc
	$(UPC) $(UPCFLAGS) -c $< -o $@

clean:
	-rm -f *.o

realclean: clean
	-rm -f *.x

distclean: realclean

