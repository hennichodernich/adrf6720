#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "rpi_threewire.h"
#include "adrf6720.h"

#define CS 0     //GPIO 17
#define SCLK 2   //GPIO 27
#define SDIO 3   //GPIO 22

int finish = 0;

void INTHandler(int dummy) {
    finish = 1;
}

int main (void)
  {  
  int regctr;
  uint16_t data;
  t_spipintriple spipins;

  t_regnamepair regsnames[18] = {
      {ADRF6720_SOFT_RESET, "SOFT_RESET"},
      {ADRF6720_ENABLES,    "ENABLES"},
      {ADRF6720_INT_DIV,    "INT_DIV"},
      {ADRF6720_FRAC_DIV,   "FRAC_DIV"},
      {ADRF6720_MOD_DIV,    "MOD_DIV"},
      {ADRF6720_ENBL_MASK,  "ENBL_MASK"},
      {ADRF6720_CP_CTL,     "CP_CTL"},
      {ADRF6720_PFD_CTL,    "PFD_CTL"},
      {ADRF6720_VCO_CTL,    "VCO_CTL"},
      {ADRF6720_BALUN_CTL,  "BALUN_CTL"},
      {ADRF6720_MOD_LIN_CTL,"MOD_LIN_CTL"},
      {ADRF6720_MOD_CTL0,   "MOD_CTL0"},
      {ADRF6720_MOD_CTL1,   "MOD_CTL1"},
      {ADRF6720_PFD_CP_CTL, "PFD_CP_CTL"},
      {ADRF6720_DITH_CTL1,  "DITH_CTL1"},
      {ADRF6720_DITH_CTL2,  "DITH_CTL2"},
      {ADRF6720_VCO_CTL2,   "VCO_CTL2"},
      {ADRF6720_VCO_CTL3,   "VCO_CTL3"}
  };


  signal(SIGINT, INTHandler);

  spipins.cs=CS;
  spipins.clk=SCLK;
  spipins.dio=SDIO;
  threewire_init(spipins);

  threewire_write16(spipins, ADRF6720_SOFT_RESET, 1);
  usleep(100000);

  for (regctr=1; regctr < 18;regctr++)
  {
    data=threewire_read16(spipins, regsnames[regctr].reg);

    printf("%02x %s:\t%04x\n",regsnames[regctr].reg,regsnames[regctr].name, data);
    if(finish)
    {
		break;
    }
    usleep(10);
  }
  if(finish)
      return;

  threewire_write16(spipins, ADRF6720_ENABLES, ADRF6720_FLAG_MOD_EN |
                    ADRF6720_FLAG_QUAD_DIV_EN | ADRF6720_FLAG_LO_1XVCO_EN |
                    ADRF6720_FLAG_VCO_MUX_EN | ADRF6720_FLAG_REF_BUF_EN |
                    ADRF6720_FLAG_VCO_EN | ADRF6720_FLAG_DIV_EN |
                    ADRF6720_FLAG_CP_EN | ADRF6720_FLAG_VCO_LDO_EN);

  threewire_write16(spipins, ADRF6720_PFD_CTL, ADRF6720_FLAG_PFD_POLARITY | ADRF6720_BITS_REF_SEL(1));

  threewire_write16(spipins, ADRF6720_VCO_CTL, ADRF6720_BITS_VCO_LDO_R4SEL(3) | ADRF6720_BITS_VCO_LDO_R2SEL(10) | ADRF6720_BITS_VCO_SEL(0));
  threewire_write16(spipins, ADRF6720_VCO_CTL3, ADRF6720_BITS_VTUNE_DAC_SLOPE(10) | ADRF6720_BITS_VTUNE_DAC_OFFSET(180));
  threewire_write16(spipins, ADRF6720_CP_CTL, ADRF6720_BITS_CP_CSCALE(15) | ADRF6720_BITS_CP_BLEED(0x20));
  threewire_write16(spipins, ADRF6720_PFD_CP_CTL, 0);

  threewire_write16(spipins, ADRF6720_INT_DIV, ADRF6720_BITS_INT_DIV(76));
  threewire_write16(spipins, ADRF6720_FRAC_DIV, (uint16_t)1);
  threewire_write16(spipins, ADRF6720_MOD_DIV, (uint16_t)4);

  usleep(10);

  finish=0;
  for (regctr=1; regctr < 18;regctr++)
  {
    data=threewire_read16(spipins, regsnames[regctr].reg);

    printf("%02x %s:\t%04x\n",regsnames[regctr].reg,regsnames[regctr].name, data);
    if(finish)
    {
        break;
    }
    usleep(10);
  }



  return 0;
  }

