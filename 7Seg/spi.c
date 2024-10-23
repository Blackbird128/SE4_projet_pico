/************************
*  SPI minimal library  *
************************/

#include <stdint.h>
#include <avr/io.h>

#include "spi.h"

// Initialisation of SPI bus
void spi_init(void)
{
SPI_DDR |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);
SPI_PORT |= (1<<SPI_SS);
SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
}

// Send byte on SPI bus
uint8_t spi_exch(uint8_t output)
{
SPDR = output;
while(!(SPSR & (1<<SPIF)));
return SPDR;
}

