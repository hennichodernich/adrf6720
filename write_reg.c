#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include "threewire.h"
#include "adrf6720.h"

static void print_usage(const char *prog)
{
    printf("Usage: %s <register> <value>\n", prog);
}

int main(int argc, char* argv[])
{
    unsigned int reg, value;
    int retval;
    t_spipintriple spipins;

    retval=threewire_init(&spipins);
    if (retval)
    {
        return(-2);
    }

    if(argc!=3)
    {
	     print_usage(argv[0]);
	      return(-1);
    }

    reg=strtol(argv[1], NULL, 16);
    value=strtol(argv[2], NULL, 16);

    printf("writing to reg %02x: %04x\n",(uint8_t)reg,(uint16_t)value);
    threewire_write16(spipins, (uint8_t)reg, (uint16_t)value);

    threewire_close(spipins);
    return(0);
}
