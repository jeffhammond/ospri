# Cray XE6
UPC      = cc -h upc
LIBS     = -lhugetlbfs
# Jeff's Mac
#UPC		 = /opt/local/upc/bin/upc 

UPCFLAGS = -g -O2 -DRELAXED
LD       = $(UPC)
LDFLAGS  = $(UPCFLAGS)

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

