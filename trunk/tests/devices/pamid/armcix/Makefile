
#CC=/bgsys/drivers/ppcfloor/comm/xl/bin/mpicc
#LD=/bgsys/drivers/ppcfloor/comm/xl/bin/mpicc

CC=/opt/ibmcmp/vacpp/bg/12.1/bin/bgxlc_r 
CFLAGS=-g -O3

LD=$(CC)
LDFLAGS=-g -O3


CFLAGS+=-I/bgsys/drivers/ppcfloor/comm/xl/include 
LDFLAGS+=-L/bgsys/drivers/ppcfloor/comm/xl/lib -lmpich -lopa -lmpl 

CFLAGS+=-I/bgsys/drivers/ppcfloor/comm/sys/include
LDFLAGS+=-L/bgsys/drivers/ppcfloor/comm/sys/lib -lpami 
#LDFLAGS+=/home/xajhammond/ARMCI-PAMI/armcix/libpami.a

CFLAGS+=-I/bgsys/drivers/ppcfloor -I/bgsys/drivers/ppcfloor/spi/include -I/bgsys/drivers/ppcfloor/spi/include/kernel/cnk
LDFLAGS+=-L/bgsys/drivers/ppcfloor/spi/lib -lSPI -lSPI_cnk -lpthread -lrt

TARGET_C=accumulate
TARGET_S=strided-accumulate

all : $(TARGET_C).cnk $(TARGET_S).cnk

$(TARGET_C).cnk : common.o armcix.o cmath.o $(TARGET_C).o 
	echo "Linking Executable $ ..."
	$(LD) -o $(TARGET_C).cnk $(TARGET_C).o common.o armcix.o cmath.o $(LDFLAGS)

$(TARGET_C).o : $(TARGET_C).c 
	$(CC) $(CFLAGS) -c $(TARGET_C).c 

$(TARGET_S).cnk : common.o armcix.o $(TARGET_S).o 
	echo "Linking Executable $ ..."
	$(LD) -o $(TARGET_S).cnk $(TARGET_S).o common.o armcix.o cmath.o $(LDFLAGS)

$(TARGET_S).o : $(TARGET_S).c 
	$(CC) $(CFLAGS) -c $(TARGET_S).c 

common.o : common.c
	$(CC) $(CFLAGS) -c common.c

cmath.o : cmath.c
	$(CC) $(CFLAGS) -c cmath.c

armcix.o : armcix.c
	$(CC) $(CFLAGS) -c armcix.c

clean:
	rm -f *.o accumulate.cnk strided-accumulate.cnk *~
