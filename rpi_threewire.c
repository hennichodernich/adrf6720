#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include "threewire.h"
#include "config_rpi.h"

int threewire_init(t_spipintriple *spipins)
{
    if (wiringPiSetup() == -1) return 1;

    (*spipins).cs=CS;
    (*spipins).clk=SCLK;
    (*spipins).dio=SDIO;

    threewire_clearpins(*spipins);

    return 0;
};

void threewire_clearpins(t_spipintriple spipins)
{
    digitalWrite(spipins.cs,1);
    pinMode(spipins.cs,  OUTPUT);

    digitalWrite(spipins.clk,0);
    pinMode(spipins.clk, OUTPUT);

    digitalWrite(spipins.dio,0);
    pinMode(spipins.dio, OUTPUT);

};

int threewire_read16(t_spipintriple spipins, uint8_t addr)
{
    int data=0;
    uint8_t addr_copy;
    int bitctr, addr_bit;;

    addr_copy = (addr << 1) | 1;
    digitalWrite(spipins.cs,0);
    for (bitctr = 0; bitctr < 8;bitctr++)
    {
      addr_bit= (addr_copy & 0x80) ? 1 : 0;
      digitalWrite(spipins.dio, addr_bit);
      usleep(1);
      digitalWrite(spipins.clk, 1);
      usleep(1);
      digitalWrite(spipins.clk, 0);
      addr_copy = addr_copy << 1;
    }
    usleep(1);
    pinMode(spipins.dio, INPUT);
    pullUpDnControl(spipins.dio, PUD_UP);
    for (bitctr = 0; bitctr < 16;bitctr++)
    {
      usleep(1);
      digitalWrite(spipins.clk, 1);
      data = data << 1;
      data |= digitalRead(spipins.dio);
      usleep(1);
          if (bitctr<15)
          digitalWrite(spipins.clk, 0);
    }
    usleep(1);
    digitalWrite(spipins.cs,1);
    usleep(1);
        digitalWrite(spipins.clk, 0);
    usleep(1);
    pinMode(spipins.dio, OUTPUT);

    return data;
};

void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data)
{
    uint16_t data_copy;
    uint8_t addr_copy;
    int bitctr, addr_bit, data_bit;

    addr_copy = (addr << 1) | 0;
    data_copy = data;
    digitalWrite(spipins.cs,0);
    for (bitctr = 0; bitctr < 8;bitctr++)
    {
      addr_bit= (addr_copy & 0x80) ? 1 : 0;
      digitalWrite(spipins.dio, addr_bit);
      usleep(1);
      digitalWrite(spipins.clk, 1);
      usleep(1);
      digitalWrite(spipins.clk, 0);
      addr_copy = addr_copy << 1;
    }
    usleep(1);

    for (bitctr = 0; bitctr < 16;bitctr++)
    {
      data_bit=(data_copy & 0x8000) ? 1 : 0;
      digitalWrite(spipins.dio, data_bit);
      usleep(1);
      digitalWrite(spipins.clk, 1);
      usleep(1);
      digitalWrite(spipins.clk, 0);
      data_copy = data_copy << 1;
    }
    usleep(1);
    digitalWrite(spipins.cs,1);
    usleep(1);
    digitalWrite(spipins.clk, 0);
    usleep(1);
    digitalWrite(spipins.dio, 0);

};

int threewire_close(t_spipintriple spipins)
{
    return(0);
}
