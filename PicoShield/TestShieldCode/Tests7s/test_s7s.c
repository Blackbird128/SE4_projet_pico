#include <avr/io.h>
#include <util/delay.h>

#define CS PD7
#define MOSI PB3
#define SCK PB5

void SPI_init() {
    DDRB |= (1 << MOSI) | (1 << SCK); // Configurer MOSI et SCK comme sorties
    DDRD |= (1 << CS); // Configurer CS comme sortie
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI_masterTransmit(uint8_t data) {
    PORTD &= ~(1 << CS);
    SPDR = data; // Charger les données
    while (!(SPSR & (1 << SPIF))); // Attendre la fin de la transmission

    PORTD |= (1 << CS);
}

int main() {
    SPI_init();

    while (1) {
         // Activer le périphérique (CS bas)
        SPI_masterTransmit(0x5B); // Envoyer le code pour '1'
         // Désactiver le périphérique (CS haut)
        _delay_ms(1000); // Attendre 1 seconde
    }
}
