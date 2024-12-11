#include "pico_serial.h"

void Serial_Init(unsigned int ubrr)
{
    /* Set baud rate */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    /* Enable receiver and transmitter */
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (3 << UCSZ00);
}

void Serial_Transmit(unsigned char data)
{
    /* Wait for empty transmit buffer */
    while (!(UCSR0A & (1 << UDRE0)));
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

unsigned char Serial_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1 << RXC0)));
    /* Get and return received data from buffer */
    return UDR0;
}

void Send_String(const char *mydata)
{
    while (*mydata)
    {
        Serial_Transmit(*mydata++);
    }
}
