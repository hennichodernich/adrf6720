#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>
#include "rpi_threewire.h"
#include "adrf6720.h"

#define CS 0     //GPIO 17
#define SCLK 2   //GPIO 27
#define SDIO 3   //GPIO 22

typedef struct {
    bool reset;
    bool dump;
    bool poweroff;
    int divider_int;
    int divider_frac;
    int divider_mod;
    bool fractional_mode;
} t_opt_struct;


int finish = 0;

void INTHandler(int dummy) {
    finish = 1;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-ifmrdo]\n", prog);
    puts("  -i --int      integer part of divider (int). Default 75.\n"
         "  -f --frac     fractional part of divider, numerator (int). Default 0.\n"
         "  -m --mod      fractional part of divider, denominator (int). Default n/a (integer mode).\n"
         "  -r --reset    reset chip first\n"
         "  -d --dump     dump registers\n"
         "  -o --poweroff power off chip\n");
    exit(1);
}

t_opt_struct parse_opts(int argc, char *argv[])
{

    const struct option lopts[] = {
        { "int",      1, 0, 'i' },
        { "frac",     1, 0, 'f' },
        { "mod",      1, 0, 'm' },
        { "reset",    0, 0, 'r' },
        { "dump",     0, 0, 'd' },
        { "poweroff", 0, 0, 'o' },
        { NULL, 0, 0, 0 },
    };
    int c;
    t_opt_struct return_opt_struct;

    return_opt_struct.dump=0;
    return_opt_struct.reset=0;
    return_opt_struct.poweroff=0;
    return_opt_struct.divider_int=75;
    return_opt_struct.divider_frac=0;
    return_opt_struct.divider_mod=0;
    return_opt_struct.fractional_mode=0;

    while (1) {

        c = getopt_long(argc, argv, "i:f:m:rdo", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'i':
            return_opt_struct.divider_int=atoi(optarg);
            break;
        case 'f':
            return_opt_struct.divider_frac=atoi(optarg);
            return_opt_struct.fractional_mode=1;
            break;
        case 'm':
            return_opt_struct.divider_mod=atoi(optarg);
            return_opt_struct.fractional_mode=1;
            break;
        case 'r':
            return_opt_struct.reset=1;
            break;
        case 'd':
            return_opt_struct.dump=1;
            break;
        case 'o':
            return_opt_struct.poweroff=1;
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }
}

int main(int argc, char* argv[])
  {  
  int regctr;
  uint16_t data, reg_intdiv, reg_fracdiv, reg_moddiv;
  t_spipintriple spipins;
  t_opt_struct program_options;

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

  program_options = parse_opts(argc, argv);

  if ((program_options.fractional_mode) && ((program_options.divider_frac==0) || (program_options.divider_mod==0)))
  {
      fprintf(stderr, "If one of --frac or --mod is given, the other one must be given as well for fractional mode.\n");
      return(-1);
  }
  if ((program_options.divider_int<1)||(program_options.divider_int>2047))
  {
      fprintf(stderr, "--int must lie between 1 and 2047.\n");
      return(-1);
  }
  if ((program_options.fractional_mode) && ((program_options.divider_frac<1)||(program_options.divider_frac>65535)||(program_options.divider_mod<1)||(program_options.divider_mod>65535)))
  {
      fprintf(stderr, "--frac and --mod must lie between 1 and 65536.\n");
      return(-1);
  }

  signal(SIGINT, INTHandler);

  spipins.cs=CS;
  spipins.clk=SCLK;
  spipins.dio=SDIO;
  threewire_init(spipins);

  if (program_options.reset)
  {
    threewire_write16(spipins, ADRF6720_SOFT_RESET, 1);
    usleep(100000);
  }

  if (program_options.dump)
  {
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
      return(-1);

  if (program_options.poweroff)
  {
      threewire_write16(spipins, ADRF6720_ENABLES, 0);

      usleep(10);
  }
  else
  {
      if(program_options.fractional_mode)
      {
          reg_intdiv = ADRF6720_BITS_INT_DIV(program_options.divider_int);
          reg_fracdiv = (uint16_t)program_options.divider_frac;
          reg_moddiv = (uint16_t)program_options.divider_mod;
      }
      else
      {
          reg_intdiv = ADRF6720_FLAG_DIV_MODE | ADRF6720_BITS_INT_DIV(program_options.divider_int);
      }

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

      threewire_write16(spipins, ADRF6720_INT_DIV, reg_intdiv);
      if(program_options.fractional_mode)
      {
        threewire_write16(spipins, ADRF6720_FRAC_DIV, reg_fracdiv);
        threewire_write16(spipins, ADRF6720_MOD_DIV, reg_moddiv);
      }
      usleep(10);

      finish=0;
      if (program_options.dump)
      {
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
  }


  return(0);
  }

