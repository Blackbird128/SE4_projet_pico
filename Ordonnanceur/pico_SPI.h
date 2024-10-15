#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#define CS      PC3  // Pin A3 (J2)
#define MOSI    PB3  // Pin 11 (MOSI)
#define MISO    PB4  // Pin 12 (MISO)
#define SCK     PB5  // Pin 13 (SCK)

void SPI_init()
{
    // set CS, MOSI and SCK to output
    DDRB |= (1 << MOSI) | (1 << SCK);
    DDRC |= (1 << CS);//J2

    // enable SPI, set as master, and clock to fosc/128
    //SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SPI_masterTransmit(uint8_t data)
{
    // load data into register
    SPDR = data;

    // Wait for transmission complete
    while(!(SPSR & (1 << SPIF)));
}

uint8_t SPI_masterReceive()
{
    // transmit dummy byte
    SPDR = 0xFF;

    // Wait for reception complete
    while(!(SPSR & (1 << SPIF)));

    // return Data Register
    return SPDR;
}

uint8_t SPI_masterTxRx(uint8_t data)
{
    // transmit data
    SPDR = data;

    // Wait for reception complete
    while(!(SPSR & (1 << SPIF)));

    // return Data Register
    return SPDR;
}
