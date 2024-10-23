/**************************************************
*  SPI display minimal library public definitions  *
***************************************************/

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "spi.h"
#include "spi_display.h"


// Select SPI 7seg display SS (Slave Select)
static void display_select(void)
{
DISPLAY_PORT &= ~(1<<DISPLAY_SEL);
}

// Unselect SPI 7seg display SS (Slave Select)
static void display_deselect(void)
{
DISPLAY_PORT |= (1<<DISPLAY_SEL);
}

// Initialize SPI 7seg display SS (Slave Select)
void spi_display_init(void){
DISPLAY_DDR |= (1<<DISPLAY_SEL);
display_deselect();
}

// Send some special command to the display
void spi_display_cmd1(uint8_t cmd){
display_select();
spi_exch(cmd);
display_deselect();
}

void spi_display_cmd2(uint8_t cmd,uint8_t arg){
display_select();
spi_exch(cmd);
spi_exch(arg);
display_deselect();
}

// Put some text on the 7seg display
void spi_display_data(uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4)
{
display_select();
spi_exch(d1);
spi_exch(d2);
spi_exch(d3);
spi_exch(d4);
display_deselect();
}
