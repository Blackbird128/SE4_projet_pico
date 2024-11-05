#include <avr/io.h>

#define DISPLAY_DDR DDRD
#define DISPLAY_PORT PORTD
#define DISPLAY_PIN 7 //Port J6 du shield

/*
 * Cette fonction déclare le pin CS de l'écran en sorti
 */
void S7S_init();

/*
 * Cette fonction envoie par SPI les 8bits passés en paramètres à l'écran
 */
void SPI_send(uint8_t data);

/*
 * Cette fonction nettoie l'écran et place le curseur à gauche
 * envoie de 0x76
 */
void clearDisplaySPI();
