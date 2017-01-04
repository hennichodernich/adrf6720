#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include "threewire.h"
#include "adrf6720.h"

typedef struct {
    int reset;
    int dump;
    int poweroff;
    int nothing;
    int divider_int;
    int divider_frac;
    int divider_mod;
    int refsel;
    int fractional_mode;
    int cp_cscale;
    int cp_bleed;
    int abldly;
    int cp_ctrl;
    int pfd_clk_edge;
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
         "  -s --refsel   clock reference selector (int). Default 1 (x1).\n"
         "  -c --cscale   charge pump coarse scale current. Default 4 (1000uA).\n"
         "  -b --bleed    charge pump bleed current. Default 32 (0uA).\n"
         "  -a --abldly   anti-backlash delay. Default 0.\n"
         "  -l --cpctrl   charge pump control. Default 0 (both on).\n"
         "  -e --clkedge  PFD clock edge. Default 0 (both down).\n"
         "  -r --reset    reset chip first\n"
         "  -d --dump     dump registers\n"
         "  -n --nothing  do nothing (useful for plain reset or dump)\n"
         "  -o --poweroff power off chip\n");
}

int parse_opts(int argc, char *argv[], t_opt_struct *opt_struct)
{

    const struct option lopts[] = {
        { "int",      1, 0, 'i' },
        { "frac",     1, 0, 'f' },
        { "mod",      1, 0, 'm' },
        { "refsel",   1, 0, 's' },
        { "cscale",   1, 0, 'c' },
        { "bleed",    1, 0, 'b' },
        { "abldly",   1, 0, 'a' },
        { "cpctrl",   1, 0, 'l' },
        { "clkedge",  1, 0, 'e' },
        { "reset",    0, 0, 'r' },
        { "dump",     0, 0, 'd' },
        { "nothing",  0, 0, 'n' },
        { "poweroff", 0, 0, 'o' },
        { NULL, 0, 0, 0 },
    };
    int c;

    (*opt_struct).dump=0;
    (*opt_struct).reset=0;
    (*opt_struct).poweroff=0;
    (*opt_struct).nothing=0;
    (*opt_struct).divider_int=75;
    (*opt_struct).divider_frac=0;
    (*opt_struct).divider_mod=0;
    (*opt_struct).fractional_mode=0;
    (*opt_struct).refsel=1;
    (*opt_struct).cp_cscale=4;
    (*opt_struct).cp_bleed=32;
    (*opt_struct).abldly=0;
    (*opt_struct).cp_ctrl=0;
    (*opt_struct).pfd_clk_edge=0;

    while (1) {

        c = getopt_long(argc, argv, "i:f:m:s:c:b:rdno", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'i':
            (*opt_struct).divider_int=atoi(optarg);
            break;
        case 'f':
            (*opt_struct).divider_frac=atoi(optarg);
            (*opt_struct).fractional_mode=1;
            break;
        case 'm':
            (*opt_struct).divider_mod=atoi(optarg);
            (*opt_struct).fractional_mode=1;
            break;
        case 's':
            (*opt_struct).refsel=atoi(optarg);
            break;
        case 'c':
            (*opt_struct).cp_cscale=atoi(optarg);
            break;
        case 'b':
            (*opt_struct).cp_bleed=atoi(optarg);
            break;
        case 'a':
            (*opt_struct).abldly=atoi(optarg);
            break;
        case 'l':
            (*opt_struct).cp_ctrl=atoi(optarg);
            break;
        case 'e':
            (*opt_struct).pfd_clk_edge=atoi(optarg);
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
        case 'o':
            (*opt_struct).poweroff=1;
            break;
        default:
            return(-1);
            break;
        }
    }
    return(0);
}

int main(int argc, char* argv[])
{
    int regctr, retval;
    uint16_t data, reg_intdiv, reg_fracdiv, reg_moddiv, reg_cpctl, reg_pfdcpctl;
    double frac_divider, f_c, f_vco;
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

    retval = parse_opts(argc, argv, &program_options);
    if (retval==-1)
    {
        print_usage(argv[0]);
        return(-2);
    }

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
    if ((program_options.refsel<0)||(program_options.refsel>4))
    {
        fprintf(stderr, "--refsel must lie between 0 and 4.\n");
        return(-1);
    }
    if ((program_options.cp_cscale<1)||(program_options.cp_cscale>4))
    {
        fprintf(stderr, "--cscale must lie between 1 and 4.\n");
        return(-1);
    }
    if ((program_options.cp_bleed<0)||(program_options.cp_bleed>63))
    {
        fprintf(stderr, "--bleed must lie between 0 and 63.\n");
        return(-1);
    }
    if ((program_options.abldly<0)||(program_options.abldly>3))
    {
        fprintf(stderr, "--abldly must lie between 0 and 3.\n");
        return(-1);
    }
    if ((program_options.cp_ctrl<0)||(program_options.cp_ctrl>4))
    {
        fprintf(stderr, "--cpctrl must lie between 0 and 4.\n");
        return(-1);
    }
    if ((program_options.pfd_clk_edge<0)||(program_options.pfd_clk_edge>3))
    {
        fprintf(stderr, "--clkedge must lie between 0 and 3.\n");
        return(-1);
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
        if(program_options.fractional_mode)
        {
            reg_intdiv = ADRF6720_BITS_INT_DIV(program_options.divider_int);
            reg_fracdiv = (uint16_t)program_options.divider_frac;
            reg_moddiv = (uint16_t)program_options.divider_mod;
            printf("setting up fractional mode with divider %d %d/%d\n",program_options.divider_int,program_options.divider_frac,program_options.divider_mod);
            frac_divider = (double)program_options.divider_int + ((double)program_options.divider_frac/(double)program_options.divider_mod);
        }
        else
        {
            reg_intdiv = ADRF6720_FLAG_DIV_MODE | ADRF6720_BITS_INT_DIV(program_options.divider_int);
            printf("setting up integer mode with divider %d\n",program_options.divider_int);
            frac_divider = (double)program_options.divider_int;

        }
        f_vco=32.0*pow(2.0,1.0-(double)program_options.refsel)*2.0*frac_divider;
        f_c = f_vco / 2.0;
        printf("f_VCO=%f, f_c=%f\n",f_vco, f_c);

        reg_cpctl = ADRF6720_BITS_CP_CSCALE(((1 << program_options.cp_cscale) - 1)) | ADRF6720_BITS_CP_BLEED(program_options.cp_bleed);
        reg_pfdcpctl = ADRF6720_BITS_ABLDLY(program_options.abldly) | ADRF6720_BITS_CP_CTRL(program_options.cp_ctrl) | ADRF6720_BITS_PFD_CLK_EDGE(program_options.pfd_clk_edge);

        threewire_write16(spipins, ADRF6720_ENABLES, ADRF6720_FLAG_MOD_EN |
                          ADRF6720_FLAG_QUAD_DIV_EN | ADRF6720_FLAG_LO_1XVCO_EN |
                          ADRF6720_FLAG_VCO_MUX_EN | ADRF6720_FLAG_REF_BUF_EN |
                          ADRF6720_FLAG_VCO_EN | ADRF6720_FLAG_DIV_EN |
                          ADRF6720_FLAG_CP_EN | ADRF6720_FLAG_VCO_LDO_EN);

        threewire_write16(spipins, ADRF6720_PFD_CTL, ADRF6720_FLAG_PFD_POLARITY | ADRF6720_BITS_REF_SEL(program_options.refsel));

        threewire_write16(spipins, ADRF6720_VCO_CTL, ADRF6720_BITS_VCO_LDO_R4SEL(3) | ADRF6720_BITS_VCO_LDO_R2SEL(10) | ADRF6720_BITS_VCO_SEL(0));
        threewire_write16(spipins, ADRF6720_VCO_CTL3, ADRF6720_BITS_VTUNE_DAC_SLOPE(10) | ADRF6720_BITS_VTUNE_DAC_OFFSET(180));
        threewire_write16(spipins, ADRF6720_CP_CTL, reg_cpctl);
        threewire_write16(spipins, ADRF6720_PFD_CP_CTL, reg_pfdcpctl);

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
    }
    threewire_close(spipins);
    return(0);
}

