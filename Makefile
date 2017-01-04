CC      = gcc
CFLAGS  = -Wall
LDFLAGS = -lm -lwiringPi
FTDI_LDFLAGS = -lm -lftdi
DUMMY_LDFLAGS = -lm

all: adrf6720
dummy: adrf6720_dummy
ftdi: adrf6720_ftdi


adrf6720: adrf6720.o rpi_threewire.o
	$(CC) $(CFLAGS) -o adrf6720 adrf6720.o rpi_threewire.o $(LDFLAGS)

adrf6720_dummy: adrf6720.o dummy_threewire.o
	$(CC) $(CFLAGS) -o adrf6720_dummy adrf6720.o dummy_threewire.o $(DUMMY_LDFLAGS)

adrf6720_ftdi: adrf6720.o ftdi_threewire.o
	$(CC) $(CFLAGS) -o adrf6720_ftdi adrf6720.o ftdi_threewire.o $(FTDI_LDFLAGS)

adrf6720.o: adrf6720.c
	$(CC) $(CFLAGS) -c $<

rpi_threewire.o: rpi_threewire.c
	$(CC) $(CFLAGS) -c $<

dummy_threewire.o: dummy_threewire.c
	$(CC) $(CFLAGS) -c $<

ftdi_threewire.o: ftdi_threewire.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -Rf adrf6720 adrf6720.o adrf6720_dummy adrf6720_ftdi rpi_threewire.o dummy_threewire.o ftdi_threewire.o


