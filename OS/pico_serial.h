#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR (FOSC / 16 / BAUD - 1)

/*
 * Initialisation de la communication série
 */
void Serial_Init(unsigned int ubrr);

/*
 * Envoie en série
 */
void Serial_Transmit(unsigned char data);

/*
 * Reception en série
 */
unsigned char Serial_Receive(void);

/*
 * Envoi d'une chaine de caractères
 */
void Send_String(const char *mydata);
