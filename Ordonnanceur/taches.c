#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "taches.h"

unsigned char serial_buffer;

//Ci dessous les differentes taches

void Led1(){
    while(1){
        PORTC ^= 0x08;
        _delay_ms(333);
    }
}

void Led2(){
    while(1){
        PORTD ^= 0x10;
        _delay_ms(400);
    }
}

void SerialWrite(){
    while (1){
        //Send_String("LOULOUTOINE"); //Test
        Serial_Transmit('\r');
        Serial_Transmit(serial_buffer);
    }
}

void SerialRead(){
    while(1){
        serial_buffer = Serial_Receive();
    }
}

void s7s(){
    int i = 0;
    while(1){
        clearDisplaySPI();
        SPI_send(i);
        SPI_send(i);
        SPI_send(i);
        SPI_send(i);
        i++;
        if (i > 15){
            i = 0;
        }
        _delay_ms(50);
    }
}

