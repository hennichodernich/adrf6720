#ifndef DUMMY_THREEWIRE_H
#define DUMMY_THREEWIRE_H

typedef struct
{
    int cs;
    int clk;
    int dio;
} t_spipintriple;

int threewire_init(t_spipintriple spipins);
void threewire_clearpins(t_spipintriple spipins);
int threewire_read16(t_spipintriple spipins, uint8_t addr);
void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data);

#endif // DUMMY_THREEWIRE_H
