#include "pico_s7s.h"

void S7S_init() {
    DISPLAY_DDR |= (1<<DISPLAY_PIN);
}

void SPI_send(uint8_t data) {
    DISPLAY_PORT &= ~(1<<DISPLAY_PIN);
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    DISPLAY_PORT |= (1<<DISPLAY_PIN);
}

void clearDisplaySPI() {
    SPI_send(0x76); // Commande pour effacer l'écran et remettre le curseur a gauche
}
