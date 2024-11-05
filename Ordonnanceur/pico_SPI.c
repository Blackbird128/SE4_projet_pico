#include "pico_SPI.h"

//Fonction pour initialiser le SPI

void SPI_init() {
    DDRB |= (1 << MOSI) | (1 << SCK) | (1<<SS);
    PORTB |= (1<<SS);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);                 // Activation SPI (SPE) en état maître (MSTR)
                                                         // horloge F_CPU/64 (SPR1=1,SPR0=0)
}
