#ifndef THREEWIRE_H
#define THREEWIRE_H

typedef struct
{
    void *context;
    int cs;
    int clk;
    int dio;
} t_spipintriple;

int threewire_init(t_spipintriple *spipins);
void threewire_clearpins(t_spipintriple spipins);
int threewire_read16(t_spipintriple spipins, uint8_t addr);
void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data);
int threewire_close(t_spipintriple spipins);

#endif // THREEWIRE_H
