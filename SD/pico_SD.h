#include <avr/io.h>
#include <string.h>
#include <stdio.h>

void SPI_init();
uint8_t SPI_transmit(uint8_t data);
uint8_t SD_send_command(uint8_t cmd, uint32_t arg);
uint8_t SD_initialize();
uint8_t SD_read_sector(uint32_t sector, uint8_t* buffer);
