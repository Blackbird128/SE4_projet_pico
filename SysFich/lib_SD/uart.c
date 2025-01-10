#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

#define BAUD 9600
#define MYUBRR (16000000 / 16 / BAUD - 1)

void UART_init()
{
	// set rate
	UBRR0H = (unsigned char)(MYUBRR >> 8);
    UBRR0L = (unsigned char)MYUBRR;

	// Enable reciever and transmitter
	UCSR0B |= (1 << RXEN0)|(1 << TXEN0);
}

void UART_putc(const unsigned char data)
{
	// wait for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0)));

	// send data to output register
	UDR0 = data;
}

void UART_puts(const char* charString)
{
	// iterate through string
	while(*charString > 0)
		// print character
		UART_putc(*charString++);
}

void UART_puthex8(uint8_t val)
{
    // extract upper and lower nibbles from input value
    uint8_t upperNibble = (val & 0xF0) >> 4;
    uint8_t lowerNibble = val & 0x0F;

    // convert nibble to its ASCII hex equivalent
    upperNibble += upperNibble > 9 ? 'A' - 10 : '0';
    lowerNibble += lowerNibble > 9 ? 'A' - 10 : '0';

    // print the characters
    UART_putc(upperNibble);
    UART_putc(lowerNibble);
}

unsigned char UART_getc(void)
{
	// wait for data to be received
	while(!(UCSR0A & (1 << RXC0)));

	// get data to output register
	return UDR0;
}

void UART_puts_p(const char* ps)
{
    register char c;

    while ((c = pgm_read_byte(ps++)))
        UART_putc(c);
}
