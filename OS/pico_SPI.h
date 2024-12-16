#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#define SS PINB2
#define MOSI PINB3
#define MISO PINB4
#define SCK PINB5

/*
 * Initialisation du SPI
 */
void SPI_init();
uint8_t SPI_transmit(uint8_t data);
