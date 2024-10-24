#include <avr/io.h>
#include <util/delay.h>

#define SS 2
#define MOSI 3
#define SCK 5

#define DISPLAY_DDR DDRD
#define DISPLAY_PORT PORTD
#define DISPLAY_PIN 7


void SPI_init() {
    DDRB |= (1 << MOSI) | (1 << SCK) | (1<<SS);
    PORTB |= (1<<SS);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);                 // Activation SPI (SPE) en état maître (MSTR)
                                                         // horloge F_CPU/64 (SPR1=1,SPR0=0)
}

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
    SPI_send(0x76); // Commande pour effacer l'affichage
}

int main(void) {
    SPI_init();
    S7S_init();
    clearDisplaySPI();
    int cmp = 0;

    while (1) {
        clearDisplaySPI();
        SPI_send(cmp);
        cmp++;
        if (cmp == 16) {
            cmp = 0;
        }
        _delay_ms(100);
    }
}