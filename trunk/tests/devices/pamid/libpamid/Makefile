ifndef COMPILER
  COMPILER = GNU
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlc_r
  COPT    = -g -qstrict --qlanglvl=extc99 -O3 -qarch=qp -qtune=qp
endif

CFLAGS  = $(COPT) -I.

OBJECTS = util.o progress.o cht.o initfinal.o barrier.o bcast.o allgather.o allreduce.o put.o get.o
TESTS = test-initfinal.x test-barrier.x test-bcast.x test-allgather.x test-allreduce.x test-put.x test-get.x

all: libpamid.a $(TESTS)

libpamid.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

%.x: %.c libpamid.a
	$(CC) $(CFLAGS) $< -L. -lpamid -o $@

%.o: %.c pamid.h pamiu.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) libpamid.a $(TESTS)

