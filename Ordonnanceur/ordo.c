#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define INT_BAS         0
#define INT_CHANGE      1
#define INT_DESCENTE    2
#define INT_MONTEE      3


#define NB_TASKS 2


#define CTC1            WGM12           // Meilleur nom pour le bit
#define PERIODE         20

int courant = 0 ;
#define portSAVE_Registers() \
    asm volatile ( \
    "push r0 \n\t" \
    "in r0, __SREG__ \n\t" \
    "push r0 \n\t" \
    "push r1 \n\t" \
    "push r2 \n\t" \
    "push r3 \n\t" \
    "push r4 \n\t" \
    "push r5 \n\t" \
    "push r6 \n\t" \
    "push r7 \n\t" \
    "push r8 \n\t" \
    "push r9 \n\t" \
    "push r10 \n\t" \
    "push r11 \n\t" \
    "push r12 \n\t" \
    "push r13 \n\t" \
    "push r14 \n\t" \
    "push r15 \n\t" \
    "push r16 \n\t" \
    "push r17 \n\t" \
    "push r18 \n\t" \
    "push r19 \n\t" \
    "push r20 \n\t" \
    "push r21 \n\t" \
    "push r22 \n\t" \
    "push r23 \n\t" \
    "push r24 \n\t" \
    "push r25 \n\t" \
    "push r26 \n\t" \
    "push r27 \n\t" \
    "push r28 \n\t" \
    "push r29 \n\t" \
    "push r30 \n\t" \
    "push r31 \n\t" \
    );


#define portRESTORE_Registers() \
asm volatile ( \
    "pop r31 \n\t" \
    "pop r30 \n\t" \
    "pop r29 \n\t" \
    "pop r28 \n\t" \
    "pop r27 \n\t" \
    "pop r26 \n\t" \
    "pop r25 \n\t" \
    "pop r24 \n\t" \
    "pop r23 \n\t" \
    "pop r22 \n\t" \
    "pop r21 \n\t" \
    "pop r20 \n\t" \
    "pop r19 \n\t" \
    "pop r18 \n\t" \
    "pop r17 \n\t" \
    "pop r16 \n\t" \
    "pop r15 \n\t" \
    "pop r14 \n\t" \
    "pop r13 \n\t" \
    "pop r12 \n\t" \
    "pop r11 \n\t" \
    "pop r10 \n\t" \
    "pop r9 \n\t" \
    "pop r8 \n\t" \
    "pop r7 \n\t" \
    "pop r6 \n\t" \
    "pop r5 \n\t" \
    "pop r4 \n\t" \
    "pop r3 \n\t" \
    "pop r2 \n\t" \
    "pop r1 \n\t" \
    "pop r0 \n\t" \
    "out __SREG__, r0 \n\t" \
    "pop r0 \n\t" \
);

typedef struct task{
  void (*start)(void);
  uint16_t stack;
}task;


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

task Taches[2]={
  {Led2, 0x0700},
  {Led1, 0x0600}
};

void init_task(int t){
  uint16_t oldSP = SP;
  SP = Taches[t].stack;
  uint16_t adresse=(uint16_t)Taches[t].start;
  asm volatile("push %0" : : "r" (adresse & 0x00ff) );
  asm volatile("push %0" : : "r" ((adresse & 0xff00)>>8) );
  portSAVE_Registers();
  Taches[t].stack = SP;
  SP = oldSP;
}

void ordonnanceur ()
{
    PORTC ^= 0x01;
	courant++;
	if (courant == NB_TASKS) courant = 0;
}

ISR(TIMER1_COMPA_vect,ISR_NAKED)
{
	/* Sauvegarde du contexte de la tâche interrompue */
	portSAVE_Registers();
	Taches[courant].stack = SP;

	/* Appel à l'ordonnanceur */
	ordonnanceur();

	/* Récupération du contexte de la tâche ré-activée */
	SP = Taches[courant].stack;
	portRESTORE_Registers();
	asm volatile ( "reti" );
}

/*ISR(TIMER1_COMPA_vect){               // Procédure d'interruption
PORTD ^= 0x0F;
}*/

int main(void){
DDRD |= 0x90;
DDRC |= 0x01;
init_minuteur(256, PERIODE);
for(int i=1;i<NB_TASKS;i++) init_task(i);
sei();                        // Autorisation des interruptions
SP = Taches[courant].stack;
Taches[courant].start();
return 0;
}
