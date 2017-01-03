#include <unistd.h>
#include <stdint.h>
#include "dummy_threewire.h"

int threewire_init(t_spipintriple spipins)
{ 
    return 0;
};

void threewire_clearpins(t_spipintriple spipins)
{

};

int threewire_read16(t_spipintriple spipins, uint8_t addr)
{
     return 0;
};

void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data)
{

};
