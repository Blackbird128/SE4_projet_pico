#include <avr/io.h>
#include <util/delay.h>

#define SS_PIN PD7 //Port J6 !!
#define MOSI_PIN PB3
#define SCK_PIN PB5


void SPI_init() {
    DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN);
    DDRD |= (1 << SS_PIN);
    PORTD |= (1 << SS_PIN);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);                 // Activation SPI (SPE) en état maître (MSTR)
                                                         // horloge F_CPU/64 (SPR1=1,SPR0=0)
}

void SPI_send(uint8_t data) {
    PORTD &= ~(1 << SS_PIN);
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    PORTD |= (1 << SS_PIN);
}

void clearDisplaySPI() {
    PORTD &= ~(1 << SS_PIN);
    SPI_send(0x76); // Commande pour effacer l'affichage
    PORTD |= (1 << SS_PIN);
}


int main(void) {
    SPI_init();
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