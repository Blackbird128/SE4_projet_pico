#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define INT_BAS         0
#define INT_CHANGE      1
#define INT_DESCENTE    2
#define INT_MONTEE      3


#define CTC1            WGM12           // Meilleur nom pour le bit
#define PERIODE         1000



void init_minuteur(int diviseur,long periode){
TCCR1A=0;               // Le mode choisi n'utilise pas ce registre
TCCR1B=(1<<CTC1);       // Réinitialisation du minuteur sur expiration
switch(diviseur){
  case    8: TCCR1B |= (1<<CS11); break;
  case   64: TCCR1B |= (1<<CS11 | 11<<CS10); break;
  case  256: TCCR1B |= (1<<CS12); break;
  case 1024: TCCR1B |= (1<<CS12 | 1<<CS10); break;
  }
// Un cycle prend 1/F_CPU secondes.
// Un pas de compteur prend diviseur/F_CPU secondes.
// Pour une periode en millisecondes, il faut (periode/1000)/(diviseur/F_CPU) pas
// soit (periode*F_CPU)/(1000*diviseur)
OCR1A=F_CPU/1000*periode/diviseur;  // Calcul du pas
TCNT1=0;                // Compteur initialisé
TIMSK1=(1<<OCIE1A);     // Comparaison du compteur avec OCR1A
}

ISR(TIMER1_COMPA_vect){               // Procédure d'interruption
PORTD ^= 0x80;
}

/*ISR(TIMER1_COMPA_vect){               // Procédure d'interruption
PORTD ^= 0x0F;
}*/

int main(void){
DDRD |= 0x80;
PORTD |= 0x80;
init_minuteur(256, PERIODE);
sei();                        // Autorisation des interruptions
while(1);
return 0;
}
