#ifdef DEBUG
#include <stdio.h>
#endif
#include <avr/io.h>
#include <stdio.h>

#ifdef DEBUG
#include "serial.h"
#endif

#include "spi.h"
#include "SdInfo.h"
#include "Sd2Card.h"

#define BLOC_NO		16
#define BLOC_TAILLE	512

int main(void)
{
#ifdef TEST_WRITE
uint8_t bloc[BLOC_TAILLE]={0xaa,0xbb,0xcc};
#endif
#ifdef TEST_READ
uint8_t bloc[BLOC_TAILLE];
#endif
#ifdef DEBUG
init_printf();
#endif
SD_info sd;
spi_init();
sd_init(&sd);
#ifdef DEBUG
printf("status=%x\n",sd.status_);
printf("erreur=%x\n",sd.errorCode_);
printf("type=%x\n",sd.type_);
uint32_t size=cardSize(&sd);
printf("taille=%ld\n",size);
#endif
#ifdef TEST_WRITE
int statut=writeBlock(&sd,BLOC_NO,bloc);
#ifdef DEBUG
printf("statut=%x\n",statut);
#endif
#endif
#ifdef TEST_READ
int statut=readBlock(&sd,BLOC_NO,bloc);
#ifdef DEBUG
printf("statut=%x\n",statut);
int i;
for(i=0;i<3;i++) printf("bloc[%d]=%x ",i,bloc[i]);
printf("\n");
#endif
#endif
return 0;
}
