/********************************************************
*  SPI 7seg display minimal library public definitions  *
*********************************************************/

#pragma once

#define	DISPLAY_DDR	DDRD
#define	DISPLAY_PORT	PORTD
#define	DISPLAY_SEL	7

void spi_display_init(void);
void spi_display_cmd1(uint8_t cmd);
void spi_display_cmd2(uint8_t cmd,uint8_t arg);
void spi_display_data(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4);
