#ifndef CONFIG_FTDI_H
#define CONFIG_FTDI_H

#undef DEADBUG

#ifdef DEADBUG

#define CS 0		//TXD, DBUS0
#define SCLK 2		//RTS, DBUS2
#define SDIO 4		//DTR, DBUS4

#else

#define CS 4		//TXD, DBUS0
#define SCLK 2		//RTS, DBUS2
#define SDIO 0		//DTR, DBUS4

#endif

#define FTDI_INTERFACE INTERFACE_A

#endif // CONFIG_FTDI_H
