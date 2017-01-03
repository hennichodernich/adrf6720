#ifndef RPI_THREEWIRE_H
#define RPI_THREEWIRE_H

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

#endif // RPI_THREEWIRE_H
