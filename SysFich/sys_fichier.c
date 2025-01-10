#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "lib_SD/uart.h"
#include "lib_SD/spi.h"
#include "lib_SD/sdcard.h"
#include "lib_SD/sdprint.h"

#include "sys_fichier.h"

//Lets go faire notre propre systeme de fichier ;)

uint8_t buffer[BLOCK_SIZE]; //variable global d'un buffer de la meme taille qu'un block de la carte SD

/*
 * Fonction de lecture du bloc passé en parametre
 * les données lues sont placées dans le buffer
 */
void lecture_block(int block){
    uint8_t res, token;
    uint32_t addr = 512 * block; //On utilise une carte SD standard (pas SDHC) il faut multiplier le numero de bloc par 512

    // Lecture de secteur
    res = SD_readSingleBlock(addr, buffer, &token);
    //UART_pputs("\r\nReponse:\r\n");

    //Lecteur de l'état de la carte SD
    //SD_printR1(res);

    // Impression en cas d'erreur
    if(!(token & 0xF0)){
        UART_pputs("Erreur :\r\n");
        SD_printDataErrToken(token);
    }
}


/*
 * Fonction d'écriture du bloc passé en parametre
 * les données à écrire doivent etre placées dans le buffer
 */
void ecriture_block(int block){
    uint8_t res, token;
    uint32_t addr = 512 * block; //On utilise une carte SD standard (pas SDHC) il faut multiplier le numero de bloc par 512

    addr = 512 * block;

    // Ecriture du buffer sur le secteur de la carte
    res = SD_writeSingleBlock(addr, buffer, &token);

    //UART_pputs("\r\nReponse:\r\n");
    //SD_printR1(res);

    // if no errors writing
    if(res == 0x00){
        if(token == SD_DATA_ACCEPTED);
        //UART_pputs("Write successful\r\n");
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

/*
 * Cette fonction renvoie l'index du premier emplacement de fichier dispo dans la TOC
 * renvoie -1 si aucun emplacement n'est disponible
 */
int first_file_available() {
    lecture_block(0);

    // Parcourir la TOC pour trouver un emplacement libre
    for (int i = 0; i < MAX_FILE; i++) {
        Fichier fichier;

        for (int j = 0; j < sizeof(Fichier); j++) {
            ((uint8_t*)&fichier)[j] = buffer[j + i * sizeof(Fichier)];
        }

        // Si le fichier est disponible (available == 0x01), l'emplacement est libre
        if (fichier.available == 0x01) {
            return i; // Retourne l'index du premier emplacement libre
        }
    }
    //Aucun emplacement libre (nombre max de fichiers atteint)
    return -1;
}

/*
 * Cette fonction clear le buffer (le rempli de 0x00)
 */
void clear_buffer(){
    for(uint16_t i = 0; i < 512; i++){
        buffer[i] = 0x00;
    }
}


/*
 * Cette fonction formate les blocks utilisés par notre systeme de fichier
 * Et rend disponible tout les fichiers dans la TOC
 */
void FORMAT(){
    UART_pputs("Formatage...\r\n");
    //On rempli le buffer de 0x00
    clear_buffer();

    for(int i = 0; i < FIRST_FILE_BLOCK + (BLOCK_PAR_FILE * MAX_FILE); i++){
        //on remplie tout les blocs de données et la TOC avec les 0x00
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
 * Cette fonction parcours la TOC et imprime sur le port serie le nom des Fichiers pour lesquels available est a 0x00
 * Si available est a 0x00 c'est que les blocs correspondants à ce fichier contiennent des données
 */
void LS(){
    UART_pputs("Fichier(s) sur la carte SD :\r\n");
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
            UART_puts(fichier.name); //Affichage du nom du fichier trouvé
            UART_pputs("\r\n");
        }
    }
}

/*
 * Cette fonction crée un fichier si il n'exite pas, si il existe les données en parametres sont ajoutées à la fin du fichier
 */
void APPEND(char *name, uint8_t *data, int taille){

    if (fichier_existe(name)) {
        // Le fichier existe on Append les données

        //TODO

        UART_pputs("Données ajoutées au fichier.\r\n");


    } else {
        //Le fichier n'existe pas on le crée (si possible)
        int index = first_file_available();
        if (index == -1){
            UART_pputs("Plus de place pour le fichier\r\n");
            return;
        }
        //Le fichier a écrire depasse la taille max
        if (taille > BLOCK_PAR_FILE * BLOCK_SIZE){
            UART_pputs("Fichier trop gros pour notre systeme de fichiers");
        }

        Fichier fichier;
        fichier.available = 0x00; // L'emplacement est maintenant occupé
        strncpy(fichier.name, name, FILE_NAME); // Copier le nom du fichier
        fichier.starting_block = FIRST_FILE_BLOCK + (index * BLOCK_PAR_FILE); // Premier bloc du fichier

        lecture_block(0);

        //Il faut ecrire le fichier au bon index du buffer
        int i = 0;
        for (int j = index * sizeof(Fichier); j < index * sizeof(Fichier) + sizeof(Fichier); j++) {
            buffer[j] = ((uint8_t*)&fichier)[i];
            i++;
        }

        // mise a jour de la TOC
        ecriture_block(0);

        clear_buffer(); //Necessaire car il contient la TOC

        // Ajouter les données dans les blocs du fichier
        int current_block = fichier.starting_block;
        int data_index = 0;
        for (int i = 0; i < taille; i++) {
            buffer[data_index++] = data[i];
            if (data_index >= BLOCK_SIZE) {
                // Si le bloc est plein, on écrit ce bloc et on passe au suivant
                ecriture_block(current_block);
                current_block++; // Passer au bloc suivant
                // Réinitialiser le buffer pour le prochain bloc
                clear_buffer();
                data_index = 0;
            }
        }
        // Ecriture du bloc courant
        if (data_index > 0) {
            ecriture_block(current_block);
        }

        UART_pputs("Fichier créé.\r\n");
    }

}

int main(void)
{
    UART_init();
    SPI_init(SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0);

    _delay_ms(500);

    //Initialisation carte SD
    if(SD_init() != SD_SUCCESS)
    {
        UART_pputs("Erreur Initialisation de la carte SD\r\n");
        return 0;
    }
    UART_pputs("Carte SD connectee\r\n");

    FORMAT();

    LS();

    APPEND("louis.test","12345678901234567890louis",25);

    lecture_block(0); //La TOC
    SD_printBuf(buffer);

    lecture_block(2); //Premier bloc avec des données
    SD_printBuf(buffer);

    LS();


    while(1){}
}
