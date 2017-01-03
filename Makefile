CC      = gcc
CFLAGS  = -Wall
LDFLAGS = -lm -lwiringPi
DUMMY_LDFLAGS = -lm

all: adrf6720
dummy: adrf6720_dummy

adrf6720: adrf6720.o rpi_threewire.o
	$(CC) $(CFLAGS) -o adrf6720 adrf6720.o rpi_threewire.o $(LDFLAGS)

adrf6720_dummy: adrf6720.o dummy_threewire.o
	$(CC) $(CFLAGS) -o adrf6720_dummy adrf6720.o dummy_threewire.o $(DUMMY_LDFLAGS)

adrf6720.o: adrf6720.c
	 $(CC) $(CFLAGS) -c $<

rpi_threewire.o: rpi_threewire.c
	 $(CC) $(CFLAGS) -c $<

dummy_threewire.o: dummy_threewire.c
	 $(CC) $(CFLAGS) -c $<

clean:
	rm -Rf adrf6720 adrf6720.o adrf6720_off adrf6720_off.o rpi_threewire.o dummy_threewire.o

