
CC=/bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc64-bgq-linux-gcc
LD=/bgsys/drivers/ppcfloor/gnu-linux/bin/powerpc64-bgq-linux-gcc
FLOOR=/bgsys/drivers/ppcfloor/
SPI_DIR=/bgsys/drivers/ppcfloor/spi/
SPI_CNK_LIB=SPI_cnk
SPI_LIB=SPI
CFLAGS=-O3 -g -I $(FLOOR) -I $(SPI_DIR)/include/kernel/cnk
TARGET_A=topology

all : $(TARGET_A)

$(TARGET_A): $(TARGET_A).o
	@echo "Linking Executable $@ ..."
	$(LD) -o $(TARGET_A).cnk $(TARGET_A).o -lpthread -static -L $(SPI_DIR)/lib -l $(SPI_LIB) -l $(SPI_CNK_LIB)


$(TARGET_A).o: $(TARGET_A).c 
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.cobaltlog *.output *.error *.o  core.*
