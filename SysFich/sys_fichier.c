#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "lib_SD/uart.h"
#include "lib_SD/spi.h"
#include "lib_SD/sdcard.h"
#include "lib_SD/sdprint.h"

#include "sys_fichier.h"

//Lets go faire notre propre systeme de fichier ;)

uint8_t buffer[BLOCK_SIZE]; //variable globale d'un buffer de la même taille qu'un bloc de la carte SD
unsigned char serial_buffer[MAX_BUFFER] = {0}; // Buffer pour stocker la chaîne reçue en UART

/*
 * Fonction de lecture du bloc passé en paramètre
 * les données lues sont placées dans le buffer
 */
void lecture_block(int block){
    uint8_t res, token;
    uint32_t addr = 512 * block; //On utilise une carte SD standard (pas SDHC) il faut multiplier le numéro de bloc par 512

    // Lecture de secteur
    res = SD_readSingleBlock(addr, buffer, &token);
    (void)res;

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

    // Ecriture du buffer sur le secteur de la carte
    res = SD_writeSingleBlock(addr, buffer, &token);

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

    //parcours de la TOC
    for (int i = 0; i < MAX_FILE; i++) {
        //remplissage de la struct Fichier
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
 * Cette fonction renvoie la struct Fichier du fichier dont le nom est passé en paramètre
 */
Fichier get_Fichier(char *name){
    // lecture de la TOC dans le buffer
    lecture_block(0);
    Fichier fichier;

    //parcours de la toc
    for (int i = 0; i < MAX_FILE; i++) {
        //remplit la struct Fichier
        for (int j = 0; j < sizeof(Fichier); j++) {
            ((uint8_t*)&fichier)[j] = buffer[j + i * sizeof(Fichier)];
        }
        if (strncmp(fichier.name, name, FILE_NAME) == 0) {
                return fichier;
        }
    }
    return fichier;
}

/*
 * Cette fonction renvoie l'index de l'emplacement de la struct Fichier passée en parametre
 */
int void_get_index_from_TOC(Fichier fichier){
    lecture_block(0);
    Fichier fichier_temp;
    for (int i = 0; i < MAX_FILE; i++) {
        for (int j = 0; j < sizeof(Fichier); j++) {
            ((uint8_t*)&fichier_temp)[j] = buffer[j + i * sizeof(Fichier)];
        }

        //Si on trouve le fichier avec le nom recherché
        if (strncmp(fichier.name, fichier_temp.name, FILE_NAME) == 0) {
            return i;
        }
    }
    return -1; //Probleme
}

/*
 * Cette fonction permet de lire une ligne par UART et la copie dans serial_buffer
 */
void serial_read_line(){
    unsigned char buffer_index = 0;
    while(1) {
        unsigned char received_char = UART_getc();
        if (received_char == '\n' || received_char == '\r') {
            serial_buffer[buffer_index] = '\0';  // Ajouter le caractère de fin de chaîne
            buffer_index = 0;  // Réinitialiser l'indice pour la prochaine lecture
            break;
        } else {
            // Stocker chaque caractère reçu dans le buffer
            serial_buffer[buffer_index++] = received_char;
            if (buffer_index >= sizeof(serial_buffer)) {
                buffer_index = sizeof(serial_buffer) - 1;
            }
        }
    }
}

/*
 * Cette fonction formate les blocks utilisés par notre système de fichier
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
    memset(fichier.name, 0x00, FILE_NAME); //nom vide (charactere 00 en hex)

    //On écrit la TOC dans le buffer (on place des structs Fichier avec available -> 0x01)
    for(int j = 0; j < MAX_FILE; j++) {

        fichier.starting_block = FIRST_FILE_BLOCK + (j * BLOCK_PAR_FILE);

        int start_index = j * sizeof(Fichier);

        for (int k = 0; k < sizeof(Fichier); k++) {
            buffer[start_index + k] = ((uint8_t*)&fichier)[k];  // copie octet par octet
        }
    }
    ecriture_block(0);
    UART_pputs("Formatage terminé\r\n");
}

/*
 * Cette fonction parcours la TOC et imprime sur le port série le nom des Fichiers pour lesquels available est à 0x00
 * Si available est à 0x00 c'est que que le fichier existe
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
 * Cette fonction crée un fichier si il n'existe pas, si il existe les données en paramètres sont ajoutées à la fin du fichier
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
        //Le fichier à écrire dépasse la taille max
        if (taille > BLOCK_PAR_FILE * BLOCK_SIZE){
            UART_pputs("Fichier trop gros pour notre systeme de fichiers");
        }

        Fichier fichier;
        fichier.available = 0x00; // L'emplacement est maintenant occupé
        strncpy(fichier.name, name, FILE_NAME); // Copier le nom du fichier
        fichier.starting_block = FIRST_FILE_BLOCK + (index * BLOCK_PAR_FILE); // Premier bloc du fichier

        lecture_block(0);

        //Il faut écrire le fichier au bon index du buffer
        int i = 0;
        for (int j = index * sizeof(Fichier); j < index * sizeof(Fichier) + sizeof(Fichier); j++) {
            buffer[j] = ((uint8_t*)&fichier)[i];
            i++;
        }

        // mise à jour de la TOC
        ecriture_block(0);

        clear_buffer(); //Nécessaire car il contient actuellement la TOC
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

/*
 * Cette fonction lit le fichier dont le nom est passé en paramètre
 * On envoie directement le contenu du fichier sur la "sortie" série car l'Atmega 328p n'a pas assez de mémoire pour stocker un fichier de taille BLOCK_SIZE * BLOCK_PAR_FILE
 */
void READ(char *name){
    Fichier fichier;
    // Si le fichier n'est pas trouvé, on quitte la fonction READ
    if (!fichier_existe(name)) {
        UART_pputs("Fichier non trouvé.\r\n");
        return;
    }
    fichier = get_Fichier(name);
    // On lit les blocs du fichier
    int current_block = fichier.starting_block;
    int run = 1;
    int colonne = 0;
    uint8_t octet;
    // Lecture des blocs jusqu'à trouver le caractère nul 0x00
    while (run) {
        lecture_block(current_block);
        for (int i = 0; i < BLOCK_SIZE; i++) {
            octet = buffer[i];
            if (octet== 0x00){
                run = 0;
            } else {
                UART_puthex8(octet);
                UART_pputs(" ");

                // Un peu de mise en page sur le retour série
                colonne ++;
                if (colonne > 19){
                    UART_pputs("\r\n");
                    colonne = 0;
                }
            }
        }
        current_block++;
    }
    UART_pputs("\r\n"); //retour chariot et retour à la ligne
}

void REMOVE(char *name){
    Fichier fichier;
    if(fichier_existe(name)){
        fichier = get_Fichier(name);
        int file_index = void_get_index_from_TOC(fichier);

        fichier.available = 0x01; // emplacement dispo
        memset(fichier.name, 0x00, FILE_NAME); //nom vide (charactere 00 en hex)

        lecture_block(0);
        // On replace le fichier vidé dans la TOC
        for (int i = 0; i < sizeof(Fichier); i++) {
            buffer[file_index + i] = ((uint8_t*)&fichier)[i];
        }

        //Ecriture de la TOC
        ecriture_block(0);

        // Plus qu'a effacer les données dans les blocs
        clear_buffer();
        for (int i = fichier.starting_block; i < fichier.starting_block + BLOCK_PAR_FILE; i++) {
            ecriture_block(i);
        }
        UART_pputs("Fichier supprimé\n\r");
    } else {
        UART_pputs("Le fichier n'existe pas\n\r");
    }
}

/*
 * Cette focntion renomme le fichier
 */
void RENAME(char *oldname, char *newname){
    Fichier fichier;
    if(fichier_existe(oldname)){
        fichier = get_Fichier(oldname);
        int file_index = void_get_index_from_TOC(fichier);

        fichier.available = 0x00;
        strncpy(fichier.name, newname, FILE_NAME); // nouveau nom

        lecture_block(0);
        // On replace le fichier renommé dans la TOC
        for (int i = 0; i < sizeof(Fichier); i++) {
            buffer[file_index + i] = ((uint8_t*)&fichier)[i];
        }

        //Ecriture de la TOC
        ecriture_block(0);

        UART_pputs("Fichier renommé\n\r");
    } else {
        UART_pputs("Le fichier n'existe pas\n\r");
    }
}

void COPY(char *source_name, char *dest_name){
    Fichier fichier;
    if(fichier_existe(source_name)){
        fichier = get_Fichier(source_name);
        int file_index = void_get_index_from_TOC(fichier);

        //todo

        UART_pputs("Fichier copié\n\r");
    } else {
        UART_pputs("Le fichier n'existe pas\n\r");
    }
}

int main(void){
    sei();
    UART_init();
    SPI_init(SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0);

    _delay_ms(500);

    //Initialisation carte SD
    if(SD_init() != SD_SUCCESS){
        UART_pputs("Erreur Initialisation de la carte SD\r\n");
        return 0;
    }
    UART_pputs("Carte SD connectée\r\n");

    while(1){
        serial_read_line(); //On lit la ligne envoyée en UART et on choisit la commande à executer
        //Facile, pas d'arguments pour LS()
        if(strcmp((char *)serial_buffer, "ls") == 0){
            LS();
        }
        //Idem pas d'arguments pour FORMAT()
        else if(strcmp((char *)serial_buffer, "format") == 0){
            FORMAT();
        }
        // READ, commande read suivie du nom du fichier
        else if(strncmp((char *)serial_buffer, "read ", 5) == 0) {
            char *filename = (char *)serial_buffer + 5;
            READ(filename);
        }
        // Commande REMOVE
        else if(strncmp((char *)serial_buffer, "remove ", 7) == 0) {
            char *filename = (char *)serial_buffer + 7; // Extraire le nom du fichier
            REMOVE(filename); // Supprimer le fichier
        }
        // Commande RENAME
        else if(strncmp((char *)serial_buffer, "rename ", 7) == 0) {
            char oldname[FILE_NAME], newname[FILE_NAME];
            if (sscanf((char *)serial_buffer + 7, "%s %s", oldname, newname) == 2) {
                RENAME(oldname, newname);
            } else {
                UART_pputs("Format de commande incorrect.\n");
            }
        }
        // Commande COPY
        else if(strncmp((char *)serial_buffer, "copy ", 5) == 0) {
            char source[FILE_NAME], destination[FILE_NAME];
            if (sscanf((char *)serial_buffer + 5, "%s %s", source, destination) == 2) {
                COPY(source, destination);
            } else {
                UART_pputs("Format de commande incorrect.\n");
            }
        }
        // Pour écrire un fichier on utilise la commande write suivie du nom du fichier à écrire
        else if(strncmp((char *)serial_buffer, "write ", 6) == 0) {
            char filename[FILE_NAME];
            strncpy(filename, (char *)serial_buffer + 6, FILE_NAME);
            UART_pputs("Entrez les données à écrire dans le fichier :\r\n");
            memset(serial_buffer, 0x00, MAX_BUFFER);
            serial_read_line(); // Lire les données à écrire dans le fichier
            APPEND(filename, serial_buffer, strlen((char *)serial_buffer)); //Ecriture des données
        }
        // Commande inconnue
        else {
            UART_pputs("Commande inconnue\r\n");
        }

        //Ajouter commande raw qui renvoie le bloc en parametre (pour du debug)
    }
    //lecture_block(0); // Verification de la TOC
    //SD_printBuf(buffer);
    //lecture_block(2); //Premier bloc avec des données
    //SD_printBuf(buffer);
    return 0;
}
