#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <ftdi.h>
#include "threewire.h"
#include "config_ftdi.h"

int threewire_init(t_spipintriple *spipins)
{
    struct ftdi_context *ftdi;
    //unsigned char buf[1];
    int f;

    (*spipins).cs=CS;
    (*spipins).clk=SCLK;
    (*spipins).dio=SDIO;


    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return -1;
    }

    (*spipins).context=(void *)ftdi;

    ftdi_set_interface(ftdi, FTDI_INTERFACE);
    f = ftdi_usb_open(ftdi, 0x0403, 0x6010);
    if (f < 0 && f != -5)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return -1;
    }
    /*
    ftdi_set_bitmode(ftdi, 0, BITMODE_BITBANG); //all input (High-Z)

    buf[0] = (1 << (*spipins).cs); // CS high, all other low
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0)
    {
        fprintf(stderr,"write failed on channel 1 for 0x%x, error %d (%s)\n", buf[0], f, ftdi_get_error_string(ftdi));
        return -1;
    }
    */

    threewire_clearpins(*spipins);

    return 0;
};

void threewire_clearpins(t_spipintriple spipins)
{
    unsigned char buf[1];
    int f;
    struct ftdi_context *ftdi;

    ftdi=(struct ftdi_context *)(spipins.context);

    //set all to output
    ftdi_set_bitmode(ftdi, (1 << spipins.cs) | (1 << spipins.clk) | (1 << spipins.dio), BITMODE_BITBANG);

    buf[0] = (1 << spipins.cs); // CS high, all other low
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");

};

int threewire_read16(t_spipintriple spipins, uint8_t addr)
{
    int data=0;
    uint8_t addr_copy;
    int bitctr, addr_bit;
    unsigned char buf[1];
    int f;
    struct ftdi_context *ftdi;

    ftdi=(struct ftdi_context *)(spipins.context);

    addr_copy = (addr << 1) | 1;    
    //digitalWrite(spipins.cs,0);
    buf[0] = 0; // all low
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");

    for (bitctr = 0; bitctr < 8;bitctr++)
    {
        addr_bit= (addr_copy & 0x80) ? 1 : 0;
        //digitalWrite(spipins.dio, (addr_copy & 0x80));
        buf[0] = (addr_bit << spipins.dio); // all low, except data pin, if 1
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        //digitalWrite(spipins.clk, 1);
        buf[0] = (addr_bit << spipins.dio) | (1 << spipins.clk); // keep CS low, keep data pin, raise CLK
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        //digitalWrite(spipins.clk, 0);
        buf[0] = (addr_bit << spipins.dio); // all low, except data pin, if 1
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");

        addr_copy = addr_copy << 1;
    }    
    usleep(1);
    //pinMode(spipins.dio, INPUT);
    //pullUpDnControl(spipins.dio, PUD_UP);
    ftdi_set_bitmode(ftdi, (1 << spipins.cs) | (1 << spipins.clk), BITMODE_BITBANG);

    for (bitctr = 0; bitctr < 16;bitctr++)
    {
      usleep(1);
      //digitalWrite(spipins.clk, 1);
      buf[0] = (1 << spipins.clk); // keep CS low, raise CLK
      f = ftdi_write_data(ftdi, buf, 1);
      if (f < 0) fprintf(stderr,"write failed\n");
      data = data << 1;
      //data |= digitalRead(spipins.dio);
      f = ftdi_read_pins(ftdi, buf);
      if (f < 0) fprintf(stderr,"read failed\n");
      data |= (buf[0] & (1 << spipins.dio)) ? 1 : 0;
      usleep(1);
          if (bitctr<15)
          {
              //digitalWrite(spipins.clk, 0);
              buf[0] = 0; // all low
              f = ftdi_write_data(ftdi, buf, 1);
              if (f < 0) fprintf(stderr,"write failed\n");
          }
    }
    usleep(1);
    //digitalWrite(spipins.cs,1);
    buf[0] = (1 << spipins.cs) | (1 << spipins.clk); // raise CS, and keep CLK high for a moment
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");
    usleep(1);
    //digitalWrite(spipins.clk, 0);
    buf[0] = (1 << spipins.cs); // now lower CLK while keeping CS
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");
    usleep(1);
    //pinMode(spipins.dio, OUTPUT);
    ftdi_set_bitmode(ftdi, (1 << spipins.cs) | (1 << spipins.clk) | (1 << spipins.dio), BITMODE_BITBANG);

    return data;
};

void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data)
{
    uint16_t data_copy;
    uint8_t addr_copy;
    int bitctr, addr_bit, data_bit;
    unsigned char buf[1];
    int f;
    struct ftdi_context *ftdi;

    ftdi=(struct ftdi_context *)(spipins.context);

    addr_copy = (addr << 1) | 0;
    data_copy = data;

    //digitalWrite(spipins.cs,0);
    buf[0] = 0; // all low
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");

    for (bitctr = 0; bitctr < 8;bitctr++)
    {
        addr_bit= (addr_copy & 0x80) ? 1 : 0;
        //digitalWrite(spipins.dio, (addr_copy & 0x80));
        buf[0] = (addr_bit << spipins.dio); // all low, except data pin, if 1
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        //digitalWrite(spipins.clk, 1);
        buf[0] = (addr_bit << spipins.dio) | (1 << spipins.clk); // keep CS low, keep data pin, raise CLK
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        //digitalWrite(spipins.clk, 0);
        buf[0] = (addr_bit << spipins.dio); // all low, except data pin, if 1
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");

        addr_copy = addr_copy << 1;
    }
    usleep(1);

    for (bitctr = 0; bitctr < 16;bitctr++)
    {
        data_bit=(data_copy & 0x8000) ? 1 : 0;
        //digitalWrite(spipins.dio, (data_copy & 0x8000));
        buf[0] = (data_bit << spipins.dio); // all low, except data pin, if 1
        f = ftdi_write_data(ftdi, buf, 1);        
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        //digitalWrite(spipins.clk, 1);
        buf[0] = (data_bit << spipins.dio) | (1 << spipins.clk); // keep CS low, keep data pin, raise CLK
        f = ftdi_write_data(ftdi, buf, 1);
        if (f < 0) fprintf(stderr,"write failed\n");
        usleep(1);
        if (bitctr<15)
        {
            //digitalWrite(spipins.clk, 0);
            buf[0] = (data_bit << spipins.dio); // all low, except data pin, if 1
            f = ftdi_write_data(ftdi, buf, 1);
            if (f < 0) fprintf(stderr,"write failed\n");
        }

        data_copy = data_copy << 1;
    }
    usleep(1);
    //digitalWrite(spipins.cs,1);
    buf[0] = (1 << spipins.cs) | (1 << spipins.clk) | (data_bit << spipins.dio); // raise CS, keep data and keep CLK high for a moment
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");
    usleep(1);

    //digitalWrite(spipins.clk, 0);
    buf[0] = (1 << spipins.cs) | (data_bit << spipins.dio); // pull CLK low
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");
    usleep(1);

    //digitalWrite(spipins.dio, 0);
    buf[0] = (1 << spipins.cs); // only keep CS high
    f = ftdi_write_data(ftdi, buf, 1);
    if (f < 0) fprintf(stderr,"write failed\n");

};

int threewire_close(t_spipintriple spipins)
{
    struct ftdi_context *ftdi;
    ftdi=(struct ftdi_context *)(spipins.context);

    ftdi_disable_bitbang(ftdi);
    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);
    return(0);
}
