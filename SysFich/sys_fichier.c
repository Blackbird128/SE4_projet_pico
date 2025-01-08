#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "uart.h"
#include "spi.h"
#include "sdcard.h"
#include "sdprint.h"

#define FILE_NAME 16
#define FIRST_FILE_BLOCK 2 // On garde les deux premiers blocs pour la TOC
#define BLOCK_PAR_FILE 4
#define MAX_FILE 8
#define BLOCK_SIZE 512

//Lets go faire notre propre systeme de fichier ;)

struct Fichier{
    uint8_t available;
    uint8_t starting_block;
    char name[FILE_NAME];
}typedef Fichier; //Taille 18 octets

uint8_t buffer[BLOCK_SIZE]; //variable global d'un buffer de la meme taille qu'un block de la carte SD

void lecture_block(int block){
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

void ecriture_block(int block){
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
 * Et rend disponible tout les fichiers dans la TOC
 */
void FORMAT(){
    //On rempli le buffer de 0x00
    for(uint16_t i = 0; i < 512; i++){
        buffer[i] = 0x00;
    }
    for(int i = 0; i < FIRST_FILE_BLOCK + (BLOCK_PAR_FILE * MAX_FILE); i++){
        //on remplie tout les blocs de données et la TOC avec les 0x00

        UART_puthex8(i);
        ecriture_block(i);
    }

    Fichier fichier; //Création d'une struct Fichier
    fichier.available = 0x01;
    strncpy(fichier.name, "                ", FILE_NAME); //nom vide (charactere 20 en hex)

    //On ecrit la TOC dans le buffer (on place des structs file avec available -> 0x01)
    for(int j = 0; j < MAX_FILE; j++) {

        fichier.starting_block = FIRST_FILE_BLOCK + (j * BLOCK_PAR_FILE);

        int start_index = j * sizeof(Fichier);

        for (int k = 0; k < sizeof(Fichier); k++) {
            buffer[start_index + k] = ((uint8_t*)&fichier)[k];  // copie octet par octet
        }
    }
    ecriture_block(0);

    UART_pputs("Formatage termine\r\n");
}

/*
 * Cette fonction parcours la TOC et imprime sur le port serie le nom des files pour lesquels available est a 0x00
 * Si available est a 0x00 c'est que les blocs correspondants a ce fichier contiennent des données
 */
void LS(){
    //On rempli le buffer avec la TOC (block 0 de la carte SD)
    lecture_block(0);

    for (int i = 0; i < MAX_FILE; i++){
        // Création d'un struct fichier
        Fichier fichier;

        // On rempli Fichier avec les infos du buffer
        for (int j = 0; j < sizeof(Fichier); j++) {
            ((uint8_t*)&fichier)[j] = buffer[j + i * sizeof(Fichier)];
        }

        // Fichier non dispo donc un fichier est stocké sur la carte
        if (fichier.available == 0x00) {
            UART_pputs("Fichier : ");
            UART_puts(fichier.name);
            UART_pputs("\r\n");
        }
    }
}

/*
 * Cette fonction crée un fichier si il n'exite pas, si il existe les données en parametres sont ajoutées à la fin du fichier
 */
void APPEND(char *name, uint8_t *data){

    if (fichier_existe(name)) {
        // Le fichier existe on Append les données

    } else {
        //LE fichier n'existe pas on le crée

    }

}

/*
 * Cette fonction renvoie 1 si le fichier passé en paramètre existe, 0 si il n'existe pas
 */
int fichier_existe(char *name) {
    // lecture de la TOC dans le buffer
    lecture_block(0);
    Fichier fichier;

    //parcours de la toc
    for (int i = 0; i < MAX_FILE; i++) {
        //remplit la struct Fichier
        for (int j = 0; j < sizeof(Fichier); j++) {
            ((uint8_t*)&fichier)[j] = buffer[j + i * sizeof(Fichier)];
        }
        if (fichier.available == 0x00) {
            if (strncmp(fichier.name, name, FILE_NAME) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    UART_init();
    SPI_init(SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0);

    _delay_ms(3000);

    //Initialisation carte SD
    if(SD_init() != SD_SUCCESS)
    {
        UART_pputs("Erreur Initialisation de la carte SD\r\n");
        return 0;
    }
    UART_pputs("Carte SD connectee\r\n");

    FORMAT();
    lecture_block(0);
    LS();
    lecture_block(0);


    while(1){}
}
