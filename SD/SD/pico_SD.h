#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdint.h>
#include "../OS/pico_SPI.h"

#define SD_PIN PIND0
#define CMD_CRC 0x94
#define CS_ENABLE()     PORTB &= ~(1 << SD_PIN)
#define CS_DISABLE()    PORTB |= (1 << SD_PIN)


void SD_powerUpSeq();
uint8_t SD_send_command(uint8_t cmd, uint32_t arg, uint8_t crc);
uint8_t SD_readRes1();
uint8_t SD_goIdleState();
uint8_t SD_initialize();
uint8_t SD_read_sector(uint32_t sector, uint8_t* buffer);
