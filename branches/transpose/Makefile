ifdef FRONTEND
    CC=xlc_r
    CFLAGS=-g -O0 -qstrict -qlanglvl=stdc99
    LIBS=safemalloc.o
else
    CC=mpixlc_r
    CFLAGS=-g -O3 -qstrict -qtune=450 -qarch=450d -qsmp=omp -qlanglvl=stdc99 -DMPI
    HPM=-L/soft/apps/UPC/lib -lhpm
    LIBS=$(HPM) safemalloc.o
endif

all: realclean transpose.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o safemalloc.o
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

ifdef FRONTEND
submit: transpose.x
	OMP_NUM_THREADS=4 ./transpose.x
else
submit: transpose.x
	qsub --mode smp -n 1 -t 55 --env OMP_NUM_THREADS=4 $< 16 3072 16
endif

clean:
	rm -f *.o 

realclean: clean
	rm -f *.x

