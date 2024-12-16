#include "pico_SPI.h"

//Fonction pour initialiser le SPI

void SPI_init() {
    DDRB |= (1 << MOSI) | (1 << SCK) | (1<<SS);
    DDRB |= (1 << MISO);
    PORTB |= (1<<SS);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);                 // Activation SPI (SPE) en état maître (MSTR)
}


uint8_t SPI_transmit(uint8_t data) {
    SPDR = data; // Mettre les données dans le registre de données SPI
    while (!(SPSR & (1 << SPIF))) { // Attendre la fin de la transmission
    }
    return SPDR; // Retourner l'octet reçu
}
