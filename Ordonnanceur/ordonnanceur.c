#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ordonnanceur.h"

#define NB_TASKS    2

int courant = 0;

task Taches[NB_TASKS] = {
  {Led2, 0x0700},
  {Led1, 0x0600}
};

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

void init_task(int t){
    uint16_t oldSP = SP;
    SP = Taches[t].stack;
    uint16_t adresse = (uint16_t)Taches[t].start;
    asm volatile("push %0" : : "r" (adresse & 0x00ff));
    asm volatile("push %0" : : "r" ((adresse & 0xff00) >> 8));
    portSAVE_Registers();
    Taches[t].stack = SP;
    SP = oldSP;
}

void ordonnanceur(){
    PORTD ^= 0x02; // On fait clignoter une des led à chaque fois que l'ordonnanceur est appelé
    courant++;
    if (courant == NB_TASKS) courant = 0;
}

void Led1(){
    while(1){
        PORTD ^= 0x80;
        _delay_ms(300);
    }
}

void Led2(){
    while(1){
        PORTD ^= 0x10;
        _delay_ms(400);
    }
}

ISR(TIMER1_COMPA_vect, ISR_NAKED){
    /* Sauvegarde du contexte de la tâche interrompue */
    portSAVE_Registers();
    Taches[courant].stack = SP;

    /* Appel à l'ordonnanceur */
    ordonnanceur();

    /* Récupération du contexte de la tâche ré-activée */
    SP = Taches[courant].stack;
    portRESTORE_Registers();
    asm volatile("reti");
}

int main(void){
    DDRD |= 0x92; //Déclaration de PD1, PD4 et PD7 en sortie
    init_minuteur(256, PERIODE);
    for(int i = 1; i < NB_TASKS; i++) init_task(i);
    sei();
    SP = Taches[courant].stack;
    Taches[courant].start();
    return 0;
}