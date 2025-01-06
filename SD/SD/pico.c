#include "pico_SD.h"



int main(void) {


    SPI_init();

    SD_powerUpSeq();

    SD_goIdleState();

    while(1);
}
