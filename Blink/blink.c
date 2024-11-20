#include <avr/io.h>
#include <util/delay.h>

int main(void){
    DDRD = 0xFF;

    while(1){
        PORTD = 0b00010000;
        _delay_ms(500);
          PORTD = 0b00100000;
        _delay_ms(500);
    }
}
