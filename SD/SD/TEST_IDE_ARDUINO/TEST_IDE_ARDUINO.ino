/*
  SD card test

*/
#include <SPI.h>
#include <SD.h>

Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 0; //Notre CS_SD est sur le pin 0 (RX)

void setup() {
  DDRD |= (1 << PD7) | (1 << PD4);
  
  for(int i=0; i < 10; i++){ //clignotement des deux led 
    PORTD ^= (1 << PD7);
    PORTD ^= (1 << PD4);
    delay(100);
  }

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    while (1){
      PORTD ^= (1 << PD7);
      delay(200); //La led de J4 clignote si l'initialisation a échouée
    }
  } else {
    PORTD |= (1 << PD7); //La led de J4 s'allume si l'initialisation de la carte SD est OK
  }


  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    while (1){ //La led de J5 s'allume si une partition est détectée 
      PORTD ^= (1 >> PD4);
      delay(200);
    }
  }
 
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
  root.close();
  PORTD |= (1 << PD4);
  
}

void loop(void) {
}
