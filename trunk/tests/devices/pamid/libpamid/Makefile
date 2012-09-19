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

OBJECTS = util.o progress.o barrier.o bcast.o allreduce.o initfinal.o 

all: realclean libpamid.a

libpamid.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

%.o: %.c pamid.h pamiu.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJECTS)

realclean: clean
	$(RM) $(RMFLAGS) libpamid.a

