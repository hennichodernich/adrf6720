CC      = gcc
CFLAGS  = -Wall -I/usr/include/libftdi1
LDFLAGS = -lm -lwiringPi
FTDI_LDFLAGS = -lm -lftdi1
DUMMY_LDFLAGS = -lm

all: adrf6720 write_reg
dummy: adrf6720_dummy
ftdi: adrf6720_ftdi write_reg_ftdi


adrf6720: adrf6720.o rpi_threewire.o
	$(CC) $(CFLAGS) -o adrf6720 adrf6720.o rpi_threewire.o $(LDFLAGS)

adrf6720_dummy: adrf6720.o dummy_threewire.o
	$(CC) $(CFLAGS) -o adrf6720_dummy adrf6720.o dummy_threewire.o $(DUMMY_LDFLAGS)

adrf6720_ftdi: adrf6720.o ftdi_threewire.o
	$(CC) $(CFLAGS) -o adrf6720_ftdi adrf6720.o ftdi_threewire.o $(FTDI_LDFLAGS)

adrf6720.o: adrf6720.c
	$(CC) $(CFLAGS) -c $<

write_reg: write_reg.o rpi_threewire.o
	$(CC) $(CFLAGS) -o write_reg write_reg.o rpi_threewire.o $(LDFLAGS)

write_reg_ftdi: write_reg.o ftdi_threewire.o
	$(CC) $(CFLAGS) -o write_reg_ftdi write_reg.o ftdi_threewire.o $(FTDI_LDFLAGS)

write_reg.o: write_reg.c
	$(CC) $(CFLAGS) -c $<


rpi_threewire.o: rpi_threewire.c
	$(CC) $(CFLAGS) -c $<

dummy_threewire.o: dummy_threewire.c
	$(CC) $(CFLAGS) -c $<

ftdi_threewire.o: ftdi_threewire.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -Rf adrf6720 adrf6720.o adrf6720_dummy adrf6720_ftdi rpi_threewire.o dummy_threewire.o ftdi_threewire.o
