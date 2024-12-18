#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)
#define MAX_BUFFER 100

// utiliser fgets pour recuperer la commande et le ou les arguments (avec leur nombre)
//puis des if pour choisir la commande demandée
unsigned char serial_buffer[MAX_BUFFER] = {0}; // Buffer pour stocker la chaîne reçue
unsigned char buffer_index = 0;


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

void Serial_Transmit_String(const char *data)
{
    while (*data) {
        Serial_Transmit(*data);  // Envoie chaque caractère
        data++;
    }
}

unsigned char Serial_Receive(void)
{
    /* Wait for data to be received */
    while (!(UCSR0A & (1 << RXC0)));
    /* Get and return received data from buffer */
    return UDR0;
}


void SerialWrite(char *data){
    while (*data){
        //Send_String("LOULOUTOINE"); //Test
        Serial_Transmit(*data);
        data++;
    }
}

void SerialRead(){
    while(1) {
        unsigned char received_char = Serial_Receive();
        if (received_char == '\n' || received_char == '\r') {
            serial_buffer[buffer_index] = '\0';  // Ajouter le caractère de fin de chaîne
            buffer_index = 0;  // Réinitialiser l'indice pour la prochaine lecture
            break;
        } else {
            // Stocker chaque caractère reçu dans le buffer
            serial_buffer[buffer_index++] = received_char;
            if (buffer_index >= sizeof(serial_buffer)) {
                buffer_index = sizeof(serial_buffer) - 1;
            }
        }
    }
}



int main(void){
    Serial_Init(MYUBRR); // Initialisation de la communication serie
    sei();
    while(1){
    SerialRead();
    if(strcmp((char *)serial_buffer, "ls") == 0){
        Serial_Transmit_String("glopglop\n\r");
    }else{
        Serial_Transmit_String("pasglop\n\r");
    }
    }
    return 0;
}


