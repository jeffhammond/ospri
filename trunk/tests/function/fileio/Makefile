CC      = mpicc
COPT    = -g -O0 -Wall

LD      = $(CC)
CFLAGS  = $(COPT)
LDFLAGS = $(COPT)
LIBS    = -lm

all: files.x

%.x: %.o
	$(LD) $(LDFLAGS) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMFLAGS) *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x

