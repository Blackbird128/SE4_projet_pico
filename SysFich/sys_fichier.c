#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "spi.h"
#include "sdcard.h"
#include "sdprint.h"

#define FILE_NAME 16
#define FIRST_FILE_BLOCK 2 //premier bloc libre apres la TOC
#define BLOCK_PAR_FILE 4
#define MAX_FILE 4
#define BLOCK_SIZE 512

//Lets go faire notre propre systeme de fichier ;)

struct file{
    char name[FILE_NAME];
    int starting_block;
    int available;
}toc;

uint8_t buffer[BLOCK_SIZE]; //variable global d'un buffer de la meme taille qu'un block de la carte SD

void test_lecture(int block){
    uint8_t res, token;
    uint32_t addr = 512 * block; //On utilise une carte SD standard (pas SDHC) il faut multiplier l'adresse par 512

    // Lecture de secteur
    res = SD_readSingleBlock(addr, buffer, &token);
    UART_pputs("\r\nReponse:\r\n");

    //Lecteur de l'état de la carte SD
    SD_printR1(res);

    // Si pas d'erreur on print le bloc
    if((res == 0x00) && (token == SD_START_TOKEN))
    SD_printBuf(buffer);
    // Si erreur on l'affiche
    else if(!(token & 0xF0))
    {
        UART_pputs("Erreur :\r\n");
        SD_printDataErrToken(token);
    }
}

void test_ecriture(int block){
    uint8_t res, token;
    uint32_t addr = 512 * block; //On utilise une carte SD standard (pas SDHC) il faut multiplier l'adresse par 512

    addr = 512 * block;

    // On remplis le buffer de 0x01
    //for(uint16_t i = 0; i < 512; i++) buffer[i] = 0x01;

    // Ecriture du buffer sur le secteur
    res = SD_writeSingleBlock(addr, buffer, &token);

    UART_pputs("\r\nReponse:\r\n");
    SD_printR1(res);

    // if no errors writing
    if(res == 0x00){
        if(token == SD_DATA_ACCEPTED)
        UART_pputs("Write successful\r\n");
    }
}

/*
 * Cette fonction formate les blocks utilisés par notre systeme de fichier
 */
void FORMAT(){
    for(uint16_t i = 0; i < 512; i++){
        buffer[i] = 0x00;
    }
    for(int i = 0; i < BLOCK_PAR_FILE * MAX_FILE; i++){
        test_ecriture(i);
    }
}

int main(void)
{
    // initialize UART
    UART_init();

    // initialize SPI
    SPI_init(SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0);

    // initialize sd card
    if(SD_init() != SD_SUCCESS)
    {
        UART_pputs("Error initializing SD CARD\r\n");
        return 0;
    }
    UART_pputs("SD Card initialized\r\n");

    test_lecture(0);
    test_ecriture(1);
    test_lecture(1);
    FORMAT();
    test_lecture(1);

    while(1) ;
}
