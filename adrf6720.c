#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include "threewire.h"
#include "frap.h"
#include "adrf6720.h"

#define NUM_REGS 25
#define WRITE_LENGTH_TX 17
#define WRITE_LENGTH_RX 19

typedef struct {
    int reset;
    int dump;
    int poweroff;
    int nothing;
    int receiver;
    int autotune;
    int optimize;
} t_opt_struct;


int finish = 0;

void INTHandler(int dummy) {
    finish = 1;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-ifmscbalerdno]\n", prog);
    puts("  -i --int      integer part of divider (int). Default 75.\n"
         "  -f --frac     fractional part of divider, numerator (int). Default 0.\n"
         "  -m --mod      fractional part of divider, denominator (int). Default n/a (integer mode).\n"
         "  -h --divider  LO divider (int). Allowed values: 1,2,4. Default 1.\n"
         "  -v --vco      VCO selector (int). Allowed values: 0,1,2,3. Default 0.\n"
         "  -s --refsel   clock reference selector (int). Default 1 (x1).\n"
         "  -k --refclk   reference clock in MHz. Default 40MHz.\n"
	 "  -x --balcin   default 0\n"
	 "  -y --balcout  default 0\n"
	 "  -z --mixbias  default 4\n"
	 "  -w --bbbias   default 2\n"
	 "  -c --rdac     default 4\n"
	 "  -u --cdac     default 1\n"
	 "  -a --attenuation attenuation in dB, range 0 to 15, default 0.\n"
	 "  -t --tune     tune to given frequency in MHz\n"
	 "  -o --optimize optimize parameters for selected frequency\n"
	 "  -j --input    0 or 1, default 0\n"
	 "  -q --qinvert  invert Q output\n"
         "  -r --reset    reset chip first\n"
         "  -d --dump     dump registers\n"
         "  -n --nothing  do nothing (useful for plain reset or dump)\n"
         "  -p --poweroff power off chip\n"
         "  -R --receiver chip is receiver ADRF6820\n");
}

int parse_opts(int argc, char *argv[], t_opt_struct *opt_struct, t_adrf6720_settings *settings)
{

    const struct option lopts[] = {
        { "int",      1, 0, 'i' },
        { "frac",     1, 0, 'f' },
        { "mod",      1, 0, 'm' },
        { "divider",  1, 0, 'h' },
        { "vco",      1, 0, 'v' },
        { "refsel",   1, 0, 's' },
        { "refclk",   1, 0, 'k' },
        { "balcin",   1, 0, 'x' },
        { "balcout",  1, 0, 'y' },
        { "mixbias",  1, 0, 'z' },
        { "bbbias",   1, 0, 'w' },
        { "rdac",     1, 0, 'c' },
        { "cdac",     1, 0, 'u' },
        { "attenuation", 1, 0, 'a' },
        { "input",    1, 0, 'j' },
        { "tune",     1, 0, 't' },
        { "optimize", 0, 0, 'o' },
        { "qinvert",  0, 0, 'q' },
        { "reset",    0, 0, 'r' },
        { "dump",     0, 0, 'd' },
        { "nothing",  0, 0, 'n' },
        { "poweroff", 0, 0, 'p' },
        { "receiver", 0, 0, 'R' },
        { NULL, 0, 0, 0 },
    };
    int c;


    while (1) {

        c = getopt_long(argc, argv, "i:f:m:h:v:s:k:x:y:z:w:c:u:a:j:t:oqrdnpR", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'i':
            (*settings).INT=atoi(optarg);
            break;
        case 'f':
            (*settings).FRAC=atoi(optarg);
            (*settings).DIV_MODE=0;
            break;
        case 'm':
            (*settings).MOD=atoi(optarg);
            (*settings).DIV_MODE=0;
            break;
        case 'h':
            (*settings).lo_divider=atoi(optarg);
            break;
        case 'v':
            (*settings).VCO_SEL=atoi(optarg);
            break;
        case 's':
            (*settings).REF_SEL=atoi(optarg);
            break;
        case 'k':
            (*settings).pll_ref_in=atof(optarg);
            break;
	case 'x':
	    (*settings).BAL_CIN=atoi(optarg);
	    break;
	case 'y':
	    (*settings).BAL_COUT=atoi(optarg);
	    break;
	case 'z':
            (*settings).MIX_BIAS=atoi(optarg);
            break;
	case 'w':
            (*settings).BB_BIAS=atoi(optarg);
            break;
	case 'c':
            (*settings).DEMOD_RDAC=atoi(optarg);
            break;
	case 'u':
            (*settings).DEMOD_CDAC=atoi(optarg);
            break;
	case 'a':
            (*settings).RFDSA_SEL=atoi(optarg);
            break;
	case 'j':
            (*settings).RFSW_SEL=atoi(optarg);
            break;
	case 't':
            (*settings).tune_freq=atof(optarg);
	    (*opt_struct).autotune = 1;
            break;
	case 'q':
	    (*settings).POLq=1;
	    break;
        case 'o':
            (*opt_struct).optimize=1;
            break;
        case 'r':
            (*opt_struct).reset=1;
            break;
        case 'd':
            (*opt_struct).dump=1;
            break;
        case 'n':
            (*opt_struct).nothing=1;
            break;
        case 'p':
            (*opt_struct).poweroff=1;
            break;
        case 'R':
            (*opt_struct).receiver=1;
            break;
        default:
            return(-1);
            break;
        }
    }
    return(0);
}

void do_tuning(t_adrf6720_settings *settings)
{
	double ideal_divider;

        settings->pll_ref_div = pow(2,(double)settings->REF_SEL-1.0);
        settings->pfd_freq = settings->pll_ref_in / settings->pll_ref_div;


	if (settings->tune_freq >= 1425)
	{
		settings->lo_divider = 1;
		settings->DIV8_EN = 0;
		settings->DIV4_EN = 0;
	}
	else if (settings->tune_freq >= 712.5)
	{
		settings->lo_divider = 2;
		settings->DIV8_EN = 0;
		settings->DIV4_EN = 1;
	}
	else
	{
		settings->lo_divider = 4;
		settings->DIV8_EN = 1;
		settings->DIV4_EN = 1;
	}

        settings->vco_freq = settings->tune_freq * (2.0 * settings->lo_divider);         //because QUAD_DIV_EN==1

	if ((settings->vco_freq < 2850) || (settings->vco_freq > 5700))
	{
		fprintf(stderr,"VCO frequency is out of range\n");
		exit(-1);
	}

	if (settings->vco_freq >= 4600)
		settings->VCO_SEL = 0;
	else if (settings->vco_freq >= 4020)
		settings->VCO_SEL = 1;
	else if (settings->vco_freq >= 3500)
		settings->VCO_SEL = 2;
	else
		settings->VCO_SEL = 3;

	ideal_divider = settings->vco_freq / (2.0 * settings->pfd_freq);

	settings->DIV_MODE=0;
        settings->INT = (int)floor(ideal_divider);
        frap(fmod(ideal_divider,1.0),2047,&(settings->FRAC),&(settings->MOD));
        if (settings->FRAC==0)
        {
        	settings->DIV_MODE=1;
        }


//        printf("f_PFD=%f, f_VCO=%f, f_c=%f\n", settings->pfd_freq, settings->vco_freq, settings->lo_out_freq);
}

void optimize(t_adrf6720_settings *settings)
{
	//values for BWSEL=0

	if (settings->tune_freq >= 1500)
		settings->BAL_CIN = 1;
	else if (settings->tune_freq >= 1300)
		settings->BAL_CIN = 2;
	else if (settings->tune_freq >= 1100)
		settings->BAL_CIN = 3;
	else if (settings->tune_freq >= 1000)
		settings->BAL_CIN = 5;
	else if (settings->tune_freq >= 900)
		settings->BAL_CIN = 6;
	else 
		settings->BAL_CIN = 7;


	if (settings->tune_freq >= 1900)
		settings->BAL_COUT = 0;
	else if (settings->tune_freq >= 1800)
		settings->BAL_COUT = 1;
	else if (settings->tune_freq >= 1700)
		settings->BAL_COUT = 0;
	else if (settings->tune_freq >= 1200)
		settings->BAL_COUT = 1;
	else if (settings->tune_freq >= 1100)
		settings->BAL_COUT = 2;
	else if (settings->tune_freq >= 1000)
		settings->BAL_COUT = 1;
	else if (settings->tune_freq >= 900)
		settings->BAL_COUT = 2;
	else if (settings->tune_freq >= 800)
		settings->BAL_COUT = 3;
	else 
		settings->BAL_COUT = 7;


	if (settings->tune_freq >= 2700)
		settings->MIX_BIAS = 1;
	else if (settings->tune_freq >= 2000)
		settings->MIX_BIAS = 2;
	else if (settings->tune_freq >= 1600)
		settings->MIX_BIAS = 1;
	else if (settings->tune_freq >= 1300)
		settings->MIX_BIAS = 2;
	else if (settings->tune_freq >= 900)
		settings->MIX_BIAS = 1;
	else
		settings->MIX_BIAS = 2;

	if (settings->tune_freq >= 2700)
		settings->DEMOD_RDAC = 8;
	else if (settings->tune_freq >= 2400)
		settings->DEMOD_RDAC = 7;
	else if (settings->tune_freq >= 2200)
		settings->DEMOD_RDAC = 9;
	else if (settings->tune_freq >= 1600)
		settings->DEMOD_RDAC = 8;
	else if (settings->tune_freq >= 1400)
		settings->DEMOD_RDAC = 9;
	else if (settings->tune_freq >= 1200)
		settings->DEMOD_RDAC = 8;
	else if (settings->tune_freq >= 1100)
		settings->DEMOD_RDAC = 9;
	else if (settings->tune_freq >= 900)
		settings->DEMOD_RDAC = 8;
	else if (settings->tune_freq >= 800)
		settings->DEMOD_RDAC = 9;
	else if (settings->tune_freq >= 700)
		settings->DEMOD_RDAC = 8;
	else
		settings->DEMOD_RDAC = 9;


	if (settings->tune_freq >= 2700)
		settings->DEMOD_CDAC = 4;
	else if (settings->tune_freq >= 2300)
		settings->DEMOD_CDAC = 3;
	else if (settings->tune_freq >= 2200)
		settings->DEMOD_CDAC = 2;
	else if (settings->tune_freq >= 2000)
		settings->DEMOD_CDAC = 4;
	else if (settings->tune_freq >= 1900)
		settings->DEMOD_CDAC = 5;
	else if (settings->tune_freq >= 1800)
		settings->DEMOD_CDAC = 6;
	else if (settings->tune_freq >= 1600)
		settings->DEMOD_CDAC = 5;
	else if (settings->tune_freq >= 1500)
		settings->DEMOD_CDAC = 4;
	else if (settings->tune_freq >= 1400)
		settings->DEMOD_CDAC = 3;
	else if (settings->tune_freq >= 1300)
		settings->DEMOD_CDAC = 7;
	else if (settings->tune_freq >= 1200)
		settings->DEMOD_CDAC = 8;
	else if (settings->tune_freq >= 1100)
		settings->DEMOD_CDAC = 6;
	else if (settings->tune_freq >= 1000)
		settings->DEMOD_CDAC = 9;
	else if (settings->tune_freq >= 900)
		settings->DEMOD_CDAC = 7;
	else if (settings->tune_freq >= 800)
		settings->DEMOD_CDAC = 4;
	else if (settings->tune_freq >= 700)
		settings->DEMOD_CDAC = 11;
	else
		settings->DEMOD_CDAC = 10;

	printf("choosing BAL_CIN=%d, BAL_COUT=%d, MIX_BIAS=%d, DEMOD_RDAC=%d, DEMOD_CDAC=%d\n",settings->BAL_CIN, settings->BAL_COUT, settings->MIX_BIAS, settings->DEMOD_RDAC, settings->DEMOD_CDAC);
}

int main(int argc, char* argv[])
{
    int regctr, retval;
    uint16_t data;
    uint16_t regs[128];
    double frac_divider;
    t_spipintriple spipins;
    t_opt_struct program_options;
    t_adrf6720_settings settings;

    t_regnamepair regsnames[NUM_REGS] = {
        {ADRF6720_SOFT_RESET, "SOFT_RESET"},
        {ADRF6720_ENABLES,    "ENABLES"},
        {ADRF6720_INT_DIV,    "INT_DIV"},
        {ADRF6720_FRAC_DIV,   "FRAC_DIV"},
        {ADRF6720_MOD_DIV,    "MOD_DIV"},
        {ADRF6720_ENBL_MASK,  "ENBL_MASK"},
        {ADRF6720_CP_CTL,     "CP_CTL"},
        {ADRF6720_PFD_CTL,    "PFD_CTL"},
        {ADRF6720_VCO_CTL,    "VCO_CTL"},
        {ADRF6720_DGA_CTL,    "DGA_CTL"},
        {ADRF6720_BALUN_CTL,  "BALUN_CTL"},
        {ADRF6720_MOD_LIN_CTL,"MOD_LIN_CTL"},
        {ADRF6720_MOD_CTL0,   "MOD_CTL0"},
        {ADRF6720_MOD_CTL1,   "MOD_CTL1"},
        {ADRF6720_MOD_CTL2,   "MOD_CTL2"},
        {ADRF6720_PFD_CP_CTL, "PFD_CP_CTL"},
        {ADRF6720_DITH_CTL1,  "DITH_CTL1"},
        {ADRF6720_DITH_CTL2,  "DITH_CTL2"},
	      {ADRF6720_CALIBRATION,"CALIBRATION"},
        {ADRF6720_VCO_CTL2,   "VCO_CTL2"},
        {ADRF6720_VCO_RB,     "VCO_RB"},
        {ADRF6720_VCO_CTL3,   "VCO_CTL3"},
        {ADRF6720_SCAN,       "SCAN"},
        {ADRF6720_VERSION1,   "VERSION1"},
        {ADRF6720_VERSION2,   "VERSION2"}
    };

    uint8_t writeorder_tx[WRITE_LENGTH_TX] = {
        ADRF6720_ENABLES,
        ADRF6720_CP_CTL,
        ADRF6720_VCO_CTL3,
	      ADRF6720_BALUN_CTL,
	      ADRF6720_MOD_LIN_CTL,
	      ADRF6720_MOD_CTL0,
	      ADRF6720_MOD_CTL1,
        ADRF6720_PFD_CP_CTL,
	      ADRF6720_DITH_CTL1,
	      ADRF6720_DITH_CTL2,
        ADRF6720_CALIBRATION,
        ADRF6720_VCO_CTL2,
        ADRF6720_PFD_CTL,
        ADRF6720_VCO_CTL,
        ADRF6720_INT_DIV,
        ADRF6720_FRAC_DIV,
        ADRF6720_MOD_DIV
    };

    uint8_t writeorder_rx[WRITE_LENGTH_RX] = {
        ADRF6720_ENABLES,
        ADRF6720_CP_CTL,
        ADRF6720_VCO_CTL3,
        ADRF6720_DGA_CTL,
	      ADRF6720_BALUN_CTL,
	      ADRF6720_MOD_LIN_CTL,
	      ADRF6720_MOD_CTL0,
	      ADRF6720_MOD_CTL1,
        ADRF6720_MOD_CTL2,
        ADRF6720_PFD_CP_CTL,
	      ADRF6720_DITH_CTL1,
	      ADRF6720_DITH_CTL2,
        ADRF6720_CALIBRATION,
        ADRF6720_VCO_CTL2,
        ADRF6720_PFD_CTL,
        ADRF6720_VCO_CTL,
        ADRF6720_INT_DIV,
        ADRF6720_FRAC_DIV,
        ADRF6720_MOD_DIV
    };

    uint8_t *writeorder;
    int write_length;

    program_options.dump=0;
    program_options.reset=0;
    program_options.poweroff=0;
    program_options.nothing=0;
    program_options.receiver=0;
    program_options.autotune=0;
    program_options.optimize=0;

    //reg 0x02
    settings.DIV_MODE=1;
    settings.INT=76;
    //reg 0x03
    settings.FRAC=384;
    //reg 0x04
    settings.MOD=1536;
    //reg 0x20
    settings.cscale_val=2;
    settings.BLEED=38;
    settings.FSCALE=0;
    //reg 0x21
    settings.REF_MUX_SEL=0;
    settings.PFD_Polarity=1;
    settings.REF_SEL=1;
    //reg 0x22
    settings.VCO_SEL=0;
    settings.LO_DRV_LVL=0;  //1x internal LO mode
    settings.DRVDIV2_EN=0;  //1x internal LO mode
    settings.lo_divider=1;
    settings.DIV8_EN=0;
    settings.DIV4_EN=0;
    settings.VCO_LDO_R2SEL=10;
    settings.VCO_LDO_R4SEL=2;
    //reg 0x23
    settings.RFSW_MUX=0; //0=software control 1=hardware control
    settings.RFSW_SEL=0;
    settings.RFDSA_SEL=0;
    //reg 0x30
    settings.BAL_COUT=0;
    settings.BAL_CIN=0;
    //reg 0x31
    settings.MIX_BIAS=4;
    settings.DEMOD_RDAC=4;
    settings.DEMOD_CDAC=1;
    settings.MOD_RSEL=(settings.MIX_BIAS << 4) | settings.DEMOD_RDAC;   //MIX_BIAS=4 and DEMOD_RDAC=4
    settings.MOD_CSEL=settings.DEMOD_CDAC;
    //reg 0x32
    settings.POLq=2;
    settings.POLi=1;
    settings.QLO=0;
    settings.ILO=0;
    //reg 0x33
    settings.DCOFFI=0;
    settings.DCOFFQ=0;
    //ref 0x34
    settings.BB_BIAS=2;
    settings.BWSEL=0;
    //reg 0x40
    settings.CLKEDGE=0;
    settings.CPCTRL=4;
    settings.ABLDLY=0;
    //reg 0x42,0x43
    settings.DITH_EN=1;
    settings.DITH_MAG=3;
    settings.DITH_VAL=0;
    //reg 0x44
    settings.SDM_DIVD_CLR=0;
    settings.BANDCAL=0;
    //reg 0x45
    settings.VTUNE_CTRL=0;
    settings.VCO_BAND_SRC=0;
    settings.BAND=0;
    //reg 0x49
    settings.VTUNE_DAC_OFFSET=0;
    settings.VTUNE_DAC_SLOPE=0;

    settings.pll_ref_in=40.0;
    settings.pll_ref_div=1.0;
    settings.lo_out_freq=2440.0;
    settings.tune_freq=0;


    retval = parse_opts(argc, argv, &program_options, &settings);
    if (retval==-1)
    {
        print_usage(argv[0]);
        return(-2);
    }

    if ((settings.DIV_MODE==0) && ((settings.FRAC==0) || (settings.MOD==0)))
    {
        fprintf(stderr, "If one of --frac or --mod is given, the other one must be given as well for fractional mode.\n");
        return(-1);
    }
    if ((settings.INT<1)||(settings.INT>2047))
    {
        fprintf(stderr, "--int must lie between 1 and 2047.\n");
        return(-1);
    }
    if ((settings.DIV_MODE==0) && ((settings.FRAC<1)||(settings.FRAC>65535)||(settings.MOD<1)||(settings.MOD>65535)))
    {
        fprintf(stderr, "--frac and --mod must lie between 1 and 65536.\n");
        return(-1);
    }

    if ((settings.lo_divider!=1) && (settings.lo_divider!=2) && (settings.lo_divider!=4))
    {
	fprintf(stderr, "--divider must be one of 1, 2 or 4.\n");
	return(-1);
    }
    else
    {

    	if (settings.lo_divider > 1)
	{
		settings.DIV4_EN = 1;
	}
	if (settings.lo_divider == 4)
	{
		settings.DIV8_EN = 1;
	}
    }

    if ((settings.VCO_SEL<0)||(settings.VCO_SEL>3))
    {
        fprintf(stderr, "--vco must lie between 0 and 3.\n");
        return(-1);
    }

    if ((settings.REF_SEL<0)||(settings.REF_SEL>4))
    {
        fprintf(stderr, "--refsel must lie between 0 and 4.\n");
        return(-1);
    }
    if ((settings.cscale_val<1)||(settings.cscale_val>4))
    {
        fprintf(stderr, "--cscale must lie between 1 and 4.\n");
        return(-1);
    }
    if ((settings.BLEED<0)||(settings.BLEED>63))
    {
        fprintf(stderr, "--bleed must lie between 0 and 63.\n");
        return(-1);
    }
    if ((settings.ABLDLY<0)||(settings.ABLDLY>3))
    {
        fprintf(stderr, "--abldly must lie between 0 and 3.\n");
        return(-1);
    }
    if ((settings.CPCTRL<0)||(settings.CPCTRL>4))
    {
        fprintf(stderr, "--cpctrl must lie between 0 and 4.\n");
        return(-1);
    }
    if ((settings.CLKEDGE<0)||(settings.CLKEDGE>3))
    {
        fprintf(stderr, "--clkedge must lie between 0 and 3.\n");
        return(-1);
    }
    if ((settings.BAL_CIN<0)||(settings.BAL_CIN>7))
    {
        fprintf(stderr, "--balcin must lie between 0 and 7.\n");
        return(-1);
    }
    if ((settings.BAL_COUT<0)||(settings.BAL_COUT>7))
    {
        fprintf(stderr, "--balcout must lie between 0 and 7.\n");
        return(-1);
    }
    if ((settings.MIX_BIAS<0)||(settings.MIX_BIAS>7))
    {
        fprintf(stderr, "--mixbias must lie between 0 and 7.\n");
        return(-1);
    }
    if ((settings.BB_BIAS<0)||(settings.BB_BIAS>3))
    {
        fprintf(stderr, "--bbbias must lie between 0 and 3.\n");
        return(-1);
    }
    if ((settings.DEMOD_RDAC<0)||(settings.DEMOD_RDAC>7))
    {
        fprintf(stderr, "--rdac must lie between 0 and 7.\n");
        return(-1);
    }
    if ((settings.DEMOD_CDAC<0)||(settings.DEMOD_CDAC>15))
    {
        fprintf(stderr, "--cdac must lie between 0 and 15.\n");
        return(-1);
    }
    if ((settings.RFDSA_SEL<0)||(settings.RFDSA_SEL>15))
    {
        fprintf(stderr, "--attenuation must lie between 0 and 15.\n");
        return(-1);
    }
    if ((settings.RFSW_SEL<0)||(settings.RFSW_SEL>1))
    {
        fprintf(stderr, "--input must lie between 0 and 1.\n");
        return(-1);
    }

    if (program_options.autotune==1)
	    if ((settings.tune_freq < 356.25) || (settings.tune_freq>2850))
	    {
	        fprintf(stderr, "--tune must lie between 326.25 and 2850.\n");
        	return(-1);
	    }

    if (program_options.receiver==1)
    {
      write_length = WRITE_LENGTH_RX;
      writeorder = writeorder_rx;
    }
    else
    {
      write_length = WRITE_LENGTH_TX;
      writeorder = writeorder_tx;
    }

    signal(SIGINT, INTHandler);

    retval=threewire_init(&spipins);
    if (retval)
    {
        return(-2);
    }

    if (program_options.reset)
    {
        printf("resetting\n");
        threewire_write16(spipins, ADRF6720_SOFT_RESET, 1);
        usleep(100000);
    }

    if (program_options.dump)
    {
        for (regctr=1; regctr < NUM_REGS;regctr++)
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
    if(finish)
        return(-1);

    if (program_options.nothing)
    {
        //nothing
    }
    else if (program_options.poweroff)
    {
        printf("powering off\n");
        threewire_write16(spipins, ADRF6720_ENABLES, 0);

        usleep(10);
    }
    else
    {
	if (program_options.autotune)
		do_tuning(&settings);

	if (program_options.optimize)
		optimize(&settings);


        if(settings.DIV_MODE==0)  //fractional mode
        {
	    while (settings.MOD < 40)
            {
            	settings.MOD  *= 2;
                settings.FRAC *= 2;
            }

            printf("setting up fractional mode with divider %d %d/%d\n",settings.INT,settings.FRAC,settings.MOD);
            regs[ADRF6720_FRAC_DIV] = (uint16_t)settings.FRAC;
            regs[ADRF6720_MOD_DIV] = (uint16_t)settings.MOD;
            frac_divider = (double)settings.INT + ((double)settings.FRAC/(double)settings.MOD);
        }
        else  //integer mode
        {
            printf("setting up integer mode with divider %d\n",settings.INT);
            regs[ADRF6720_FRAC_DIV] = 0;
            regs[ADRF6720_MOD_DIV] = 0;
            frac_divider = (double)settings.INT;
        }
        regs[ADRF6720_INT_DIV] = (settings.DIV_MODE ? ADRF6720_FLAG_DIV_MODE : 0)
                              |  ADRF6720_BITS_INT_DIV(settings.INT);

        //hardcoded for carrier frequency

        regs[ADRF6720_VCO_CTL] = ADRF6720_BITS_VCO_LDO_R4SEL(settings.VCO_LDO_R4SEL)
                               | ADRF6720_BITS_VCO_LDO_R2SEL(settings.VCO_LDO_R2SEL)
                               | ADRF6720_BITS_LO_DRV_LVL(settings.LO_DRV_LVL)
                               |(settings.DRVDIV2_EN ? ADRF6720_FLAG_DRVDIV2_EN : 0 )
                               |(settings.DIV8_EN ? ADRF6720_FLAG_DIV8_EN : 0 )
                               |(settings.DIV4_EN ? ADRF6720_FLAG_DIV4_EN : 0 )
                               | ADRF6720_BITS_VCO_SEL(settings.VCO_SEL);

        regs[ADRF6720_DGA_CTL] = (settings.RFSW_MUX ? ADRF6720_FLAG_RFSW_MUX : 0 )
                               | (settings.RFSW_SEL ? ADRF6720_FLAG_RFSW_SEL : 0 )
                               | ADRF6720_BITS_RFDSA_SEL(settings.RFDSA_SEL);

        regs[ADRF6720_PFD_CTL] =  ADRF6720_BITS_REF_MUX_SEL(settings.REF_MUX_SEL)
                               | (settings.PFD_Polarity ? ADRF6720_FLAG_PFD_POLARITY : 0 )
                               |  ADRF6720_BITS_REF_SEL(settings.REF_SEL);

        settings.pll_ref_div = pow(2,(double)settings.REF_SEL-1.0);

        settings.pfd_freq = settings.pll_ref_in / settings.pll_ref_div;
        settings.vco_freq = frac_divider * 2.0 * settings.pfd_freq;
        settings.lo_out_freq = settings.vco_freq / (2.0 * settings.lo_divider);         //because QUAD_DIV_EN==1

        printf("f_PFD=%f, f_VCO=%f, f_c=%f\n", settings.pfd_freq, settings.vco_freq, settings.lo_out_freq);

/*
        regs[ADRF6720_VCO_CTL3] = ADRF6720_BITS_VTUNE_DAC_SLOPE(settings.VTUNE_DAC_SLOPE)
                                | ADRF6720_BITS_VTUNE_DAC_OFFSET(settings.VTUNE_DAC_OFFSET);
*/

        regs[ADRF6720_VCO_CTL3] = 0x14B4; //according to data sheet

        regs[ADRF6720_SCAN] = settings.SCAN_EN ? ADRF6720_FLAG_SCAN_EN : 0;

        regs[ADRF6720_VCO_CTL2] = ADRF6720_BITS_VTUNE_CTRL(settings.VTUNE_CTRL)
                                | (settings.VCO_BAND_SRC ? ADRF6720_FLAG_VCO_BAND_SRC : 0 )
                                | ADRF6720_BITS_BAND(settings.VCO_BAND);

        regs[ADRF6720_CALIBRATION] = (settings.BANDCAL ? ADRF6720_FLAG_BANDCAL : 0 )
                                   | (settings.SDM_DIVD_CLR ? ADRF6720_FLAG_SDM_DIVD_CLR : 0);

        regs[ADRF6720_DITH_CTL1] = (settings.DITH_EN ? ADRF6720_FLAG_DITH_EN : 0 )
                                 |  ADRF6720_BITS_DITH_MAG(settings.DITH_MAG)
                                 | ((settings.DITH_VAL >= 65536) ? ADRF6720_BITS_DITH_VAL16 : 0);

        regs[ADRF6720_DITH_CTL2] = settings.DITH_VAL % 65536;

        regs[ADRF6720_PFD_CP_CTL] = ADRF6720_BITS_ABLDLY(settings.ABLDLY)
                                  | ADRF6720_BITS_CP_CTRL(settings.CPCTRL)
                                  | ADRF6720_BITS_PFD_CLK_EDGE(settings.CLKEDGE);

        regs[ADRF6720_MOD_CTL0] = ADRF6720_BITS_POL_I(settings.POLi)
                                | ADRF6720_BITS_POL_Q(settings.POLq)
                                | ADRF6720_BITS_I_LO(settings.ILO)
                                | ADRF6720_BITS_Q_LO(settings.QLO);


        regs[ADRF6720_MOD_CTL1] = ADRF6720_BITS_DCOFF_I(settings.DCOFFI)
                                | ADRF6720_BITS_DCOFF_Q(settings.DCOFFQ);

        regs[ADRF6720_MOD_CTL2] = ADRF6720_BITS_BB_BIAS(settings.BB_BIAS)
                                | ADRF6720_BITS_BWSEL(settings.BWSEL);


        regs[ADRF6720_MOD_LIN_CTL] = ADRF6720_BITS_MOD_RSEL(settings.MOD_RSEL)
                                  | ADRF6720_BITS_MOD_CSEL(settings.MOD_CSEL);

        regs[ADRF6720_CP_CTL] = ADRF6720_BITS_CP_CSCALE(((1 << settings.cscale_val) - 1))
                              | ADRF6720_BITS_CP_FSCALE(settings.FSCALE)
                              | ADRF6720_BITS_CP_BLEED(settings.BLEED);

        regs[ADRF6720_BALUN_CTL] = ADRF6720_BITS_BAL_COUT(settings.BAL_COUT)
                                 | ADRF6720_BITS_BAL_CIN(settings.BAL_CIN);

        regs[ADRF6720_ENABLES] = ADRF6720_FLAG_MOD_EN         //bit 10      0x067e
                               | ADRF6720_FLAG_QUAD_DIV_EN    //bit 9
                               | ADRF6720_FLAG_VCO_MUX_EN     //bit 6
                               | ADRF6720_FLAG_REF_BUF_EN     //bit 5
                               | ADRF6720_FLAG_VCO_EN         //bit 4
                               | ADRF6720_FLAG_DIV_EN         //bit 3
                               | ADRF6720_FLAG_CP_EN          //bit 2
                               | ADRF6720_FLAG_VCO_LDO_EN;    //bit 1


        for (regctr=0; regctr < write_length;regctr++)
        {
#if 0
	    printf("writing %x=%x\n", writeorder[regctr], regs[writeorder[regctr]]);
#endif
            threewire_write16(spipins, writeorder[regctr], regs[writeorder[regctr]]);
        }
        usleep(10);

        finish=0;
        if (program_options.dump)
        {
            for (regctr=1; regctr < NUM_REGS;regctr++)
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
    }
    threewire_close(spipins);
    return(0);
}
