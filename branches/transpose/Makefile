CC=mpixlc_r
CFLAGS=-g -O5 -qnoipa -qtune=450 -qarch=450d -qsmp=omp -qlanglvl=stdc99
HPM=-L/soft/apps/UPC/lib -lhpm
LIBS=$(HPM)

all: transpose.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

submit: transpose.x
	qsub --mode smp -n 1 -t 10 --env OMP_NUM_THREADS=4 $<

clean:
	rm -f *.o 

realclean: clean
	rm -f *.x

