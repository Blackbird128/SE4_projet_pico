#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#define SPI_DDR DDRB
#define CS      PINB2
#define MOSI    PINB3
#define MISO    PINB4
#define SCK     PINB5

void SPI_init()
{
    // set CS, MOSI and SCK to output
    SPI_DDR |= (1 << CS) | (1 << MOSI) | (1 << SCK);

    // enable SPI, set as master, and clock to fosc/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

void SPI_masterTransmitByte(uint8_t data)
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

 // drive slave select low
    //SPI_DDR &= ~(1 << SS);

    // transmit byte to slave (and ignore response)
    //SPI_masterTransmit(0x55);

    // receive byte from slave
    //uint8_t ret = SPI_masterTxRx(0xFF);

    // return slave select to high
    //SPI_DDR |= (1 << SS);