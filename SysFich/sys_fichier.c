#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "lib_SD/uart.h"
#include "lib_SD/spi.h"
#include "lib_SD/sdcard.h"
#include "lib_SD/sdprint.h"

#include "sys_fichier.h"

//Lets go faire notre propre système de fichiers ;)

uint8_t buffer[BLOCK_SIZE]; //variable globale d'un buffer de la même taille qu'un bloc de la carte SD
unsigned char serial_buffer[MAX_BUFFER] = {0}; // Buffer pour stocker la chaîne reçue en UART

/*
 * Fonction de lecture du bloc passé en paramètre
 * Les données lues sont placées dans le buffer global
 * @param block : le numéro du secteur à lire
 */
void lecture_block(uint32_t block){
    uint8_t res, token;
    /*
     On utilise une carte SD standard (pas SDHC) il faut multiplier le numéro de bloc par 512 (décalage de 9bits)
     En effet les cartes SD sont bytes-addressed, le bloc 0 est à l'adresse 0, le bloc 1 à l'adresse 512...
     Le cartes SDHC et SDXC sont block-addressed, le bloc 0 est à l'adresse 0, le bloc 1 à l'adresse 1...
     */
    uint32_t addr = block << 9;
    // Lecture du bloc (secteur)
    res = SD_readSingleBlock(addr, buffer, &token);
    (void)res; //On utilise pas res (il contient la réponse de la carte SD);
    // Impression en cas d'erreur
    if(!(token & 0xF0)){
        UART_pputs("Erreur :\r\n");
        SD_printDataErrToken(token);
    }
}

/*
 * Fonction d'écriture du bloc passé en paramètre
 * Les données à écrire doivent être placées dans le buffer
 * @param block : le numero du secteur à écrire
 */
void ecriture_block(uint32_t block){
    uint8_t res, token;
    uint32_t addr = block << 9; // Idem que pour lecture_block

    // Ecriture du buffer sur le secteur de la carte
    res = SD_writeSingleBlock(addr, buffer, &token);
    (void)res;
}

/*
 * Cette fonction vérifie si un fichier est présent dans la TOC
 * @param name : le nom du fichier fichier
 * @return : 1 si le fichier existe, 0 sinon
 */
int fichier_existe(char *name) {
    Fichier fichier;
    // Parcours des deux blocs de la TOC
    for (int toc = 0; toc < TOC_BLOCKS; toc++) {
        lecture_block(toc);
        // Parcours des fichiers dans la TOC
        for (int i = 0; i < FILE_PAR_TOC; i++) {
            // Remplissage de la structure Fichier à partir du buffer
            memcpy(&fichier, &buffer[i * sizeof(Fichier)], sizeof(Fichier));
            if (fichier.available == 0x00 && strncmp(fichier.name, name, FILE_NAME) == 0) {
                return 1; // Fichier trouvé
            }
        }
    }
    return 0; // Fichier non trouvé
}

/*
 * Cette fonction trouve le premier emplacement disponible dans la table of content
 * @return l'index du premier emplacement libre dans notre système de fichiers, -1 si aucun disponible
 */
int first_file_available() {
    // Parcourir les deux TOC
    for (int toc = 0; toc < TOC_BLOCKS; toc++){
        lecture_block(toc);
        for (int i = 0; i < FILE_PAR_TOC; i++) {
            // On regarde directement les emplacements des champs "available" dans la TOC (economise l'extraction des données dans une struct Fichier utilisée dans la version precedente)
            if (buffer[i * sizeof(Fichier)] == 0x01) {
                return i + (toc * FILE_PAR_TOC); // Retourne l'index du premier emplacement libre
            }
        }
    }
    //Aucun emplacement libre (nombre max de fichiers atteint)
    return -1;
}

/*
 * Cette fonction clear le buffer (le remplit de 0x00)
 */
void clear_buffer(){
    memset(buffer, 0, sizeof(buffer));
}

/*
 * Cette fonction renvoie la struct Fichier du fichier dont le nom est passé en paramètre
 * @param name : le nom du fichier à trouver
 * @return la structure Fichier cherchée (ou un Fichier vide si il n'existe pas)
 */
Fichier get_Fichier(char *name){
    // lecture de la TOC dans le buffer
    Fichier fichier;
    for (int toc = 0; toc < TOC_BLOCKS; toc ++){
        //parcours des TOC
        for (int i = 0; i < FILE_PAR_TOC; i++) {
            lecture_block(toc);
            //remplit la struct Fichier
            memcpy(&fichier, &buffer[i * sizeof(Fichier)], sizeof(Fichier));
            // Comparaison des noms
            if (strncmp(fichier.name, name, FILE_NAME) == 0) {
                    return fichier;
            }
        }
    }
    return fichier; //Fichier vide
}

/*
 * Cette fonction imprime un bloc complet
 * Permet de debug
 * @param block : le numéro du bloc à imprimer
 */
void print_block(int block){
    lecture_block(block);
    SD_printBuf(buffer);
}

/*
 * Cette fonction renvoie l'index de l'emplacement de la struct Fichier passée en paramètre
 * @param fichier : une struct Fichier
 * @return l'index (dans la TOC) de la struct passée en paramètre
 */
int get_index_from_TOC(Fichier fichier){
    Fichier fichier_temp;
    
    for (int toc = 0; toc < TOC_BLOCKS; toc++){
        lecture_block(toc);
        for (int i = 0; i < FILE_PAR_TOC; i++) {
            memcpy(&fichier_temp, &buffer[i * sizeof(Fichier)], sizeof(Fichier));
            //Si on trouve le fichier avec le nom recherché
            if (strncmp(fichier.name, fichier_temp.name, FILE_NAME) == 0) {
                return i + (toc * FILE_PAR_TOC);
            }
        }
    }
    return -1; // Problème
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
 * Si l'index passé en paramètre est inférieur au nombre de fichiers par TOC on est dans la premiere TOC (bloc 0)
 * Si l'index est supérieur à FILE_PAR_TOC alors il faudra modifier la deuxieme TOC (dans le bloc 1)
 * @param index : l'index dont il faut déterminer la TOC
 * @return int le numéro de la TOC ou est l'index
 */
int get_toc_from_index(int index) {
    return (index < FILE_PAR_TOC) ? 0 : 1;
}

/*
 * Cette fonction formate les blocs utilisés par notre système de fichier
 * Et rend disponible tout les fichiers dans la TOC
 */
void FORMAT(){
    UART_pputs("Formatage...\r\n");
    //On remplit le buffer de 0x00
    clear_buffer();
    for(int i = 0; i < TOC_BLOCKS + (BLOCK_PAR_FILE * MAX_FILE); i++){
        //on remplit tous les blocs de données et la TOC avec les 0x00
        ecriture_block(i);
    }
    Fichier fichier; //Création d'une struct Fichier
    fichier.available = 0x01;
    memset(fichier.name, 0x00, FILE_NAME); //nom vide (charactere 00 en hex)

    //On écrit la TOC dans le buffer (on place des structs Fichier avec available -> 0x01)
    for (int toc = 0; toc < TOC_BLOCKS; toc++) {
        for (int j = 0; j < MAX_FILE; j++) {
            fichier.starting_block = TOC_BLOCKS + (j * BLOCK_PAR_FILE) + (toc * FILE_PAR_TOC);
            int start_index = j * sizeof(Fichier);
            memcpy(&buffer[start_index], &fichier, sizeof(Fichier)); // Copie de la structure
        }
        ecriture_block(toc); // Écriture du buffer
    }
    UART_pputs("Formatage terminé\r\n");
}

/*
 * Cette fonction parcours la TOC et imprime sur le port série le nom des Fichiers pour lesquels available est à 0x00
 * Si available est à 0x00 c'est que que le fichier existe
 */
void LS(){
    UART_pputs("Fichier(s) sur la carte SD :\r\n");
    // On boucle sur les differents blocs de TOC
    for (int toc = 0; toc < TOC_BLOCKS; toc++){
        lecture_block(toc);
        for (int i = 0; i < FILE_PAR_TOC; i++){
            // Création d'un struct fichier
            Fichier fichier;

            // On remplit Fichier avec les infos du buffer
            memcpy(&fichier, &buffer[i * sizeof(Fichier)], sizeof(Fichier));

            // Fichier non dispo donc un fichier est stocké sur la carte
            if (fichier.available == 0x00) {
                UART_puts(fichier.name); //Affichage du nom du fichier trouvé
                UART_pputs("\r\n");
            }
        }
    }
}

/*
 * Cette fonction crée un fichier si il n'existe pas, si il existe les données en paramètres sont ajoutées à la fin du fichier
 */
void APPEND(char *name, uint8_t *data, int taille){
    if (fichier_existe(name)) {
        // Le fichier existe on Append les données
        Fichier fichier = get_Fichier(name);

        int current_block = fichier.starting_block;
        int data_index = 0;

        for (int i = 0; i < BLOCK_PAR_FILE; i++) {
            lecture_block(current_block);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                if (buffer[j] == 0x00 && data_index < taille) {
                    buffer[j] = data[data_index++];
                }
            }
            ecriture_block(current_block);
            // les données sont totalement ajoutées
            if (data_index >= taille) {
                break;
            }
            current_block++;
        }
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
            return;
        }

        Fichier fichier;
        fichier.available = 0x00; // L'emplacement est maintenant occupé
        strncpy(fichier.name, name, FILE_NAME); // Copier le nom du fichier
        fichier.starting_block = TOC_BLOCKS + (index * BLOCK_PAR_FILE); // Premier bloc du fichier
        
        int toc = get_toc_from_index(index); // On cherche la TOC 0 ou 1
        int index_reel = index % FILE_PAR_TOC;
        lecture_block(toc);
        // On copie le fichier dans la bonne TOC
        memcpy(&buffer[index_reel * sizeof(Fichier)], &fichier, sizeof(Fichier));
        ecriture_block(toc);

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
                clear_buffer(); // Réinitialisation du buffer pour le prochain bloc
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
    // Si le fichier n'est pas trouvé, on quitte la fonction READ
    if (!fichier_existe(name)) {
        UART_pputs("Fichier non trouvé.\r\n");
        return;
    }
    Fichier fichier = get_Fichier(name);
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

/*
 * Cette fonction supprime le fichier dont le nom est passé en paramètre
 * Pour cela on replace sa struct Fichier dans la TOC par une Struct fichier "vierge" puis on remplit les blocs de données correspondant de 0x00
 */
void REMOVE(char *name){
    if(fichier_existe(name)){
        Fichier fichier = get_Fichier(name);
        int index = get_index_from_TOC(fichier);

        fichier.available = 0x01; // emplacement dispo
        memset(fichier.name, 0x00, FILE_NAME); //nom vide (charactere 00 en hex)
        
        // On determine quelle TOC lire
        int toc = get_toc_from_index(index);
        int index_reel = index % FILE_PAR_TOC;
        lecture_block(toc);
        // On replace le fichier vidé dans la TOC
        memcpy(&buffer[index_reel * sizeof(Fichier)], &fichier, sizeof(Fichier));
        ecriture_block(toc); //Ecriture de la TOC correspondante
        
        // Plus qu'à effacer les données dans les blocs
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
 * Cette fonction renomme le fichier
 */
void RENAME(char *oldname, char *newname){
    if(fichier_existe(oldname)){
        Fichier fichier = get_Fichier(oldname);
        int index = get_index_from_TOC(fichier);

        fichier.available = 0x00;
        strncpy(fichier.name, newname, FILE_NAME); // nouveau nom

        // On determine quelle TOC lire
        int toc = get_toc_from_index(index);
        int index_reel = index % FILE_PAR_TOC;
        lecture_block(toc);
        
        // On replace le fichier renommé dans la TOC
        memcpy(&buffer[index_reel * sizeof(Fichier)], &fichier, sizeof(Fichier));
        //Ecriture de la TOC
        ecriture_block(toc);
        
        UART_pputs("Fichier renommé\n\r");
    } else {
        UART_pputs("Le fichier n'existe pas\n\r");
    }
}

/*
 * Cette fonction copy un fichier source vers une nouvelle destination
 * Les blocs de données sont copiés et associés à une entrée dans la TOC
 */
void COPY(char *source_name, char *dest_name){
    Fichier fichier_source;
    Fichier fichier_dest;
    if (fichier_existe(source_name)){
        fichier_source = get_Fichier(source_name);
        if (fichier_existe(dest_name)) {
            UART_pputs("Le fichier de destination existe déjà\r\n");
            return;
        }
        // On cherche le premier emplacement libre dans la TOC
        int index_dest = first_file_available();
        if (index_dest == -1) {
            UART_pputs("Pas de place disponible pour copier le fichier\r\n");
            return;
        }
        fichier_dest.available = 0x00;
        strncpy(fichier_dest.name, dest_name, FILE_NAME);
        fichier_dest.starting_block = TOC_BLOCKS + (index_dest * BLOCK_PAR_FILE);

        // On determine quelle TOC lire
        int toc = get_toc_from_index(index_dest);
        int index_reel = index_dest % FILE_PAR_TOC;
        lecture_block(toc);
        // maj de la TOC
        memcpy(&buffer[index_reel * sizeof(Fichier)], &fichier_dest, sizeof(Fichier));
        ecriture_block(toc);
        
        // Copie des données des blocs
        int first_block_source = fichier_source.starting_block;
        int first_block_dest = fichier_dest.starting_block;
        for(int j = 0; j < BLOCK_PAR_FILE; j++){
            lecture_block(first_block_source + j);
            ecriture_block(first_block_dest + j);
        }
        UART_pputs("Fichier copié\n\r");
    } else {
        UART_pputs("Le fichier n'existe pas\n\r");
    }
}

/*
 * Cette fonction renvoie le nombre d'emplacement de fichiers disponibles
 */
void QUOTADISK(){
    int quota = 0;
    char str[10];
    for (int toc = 0; toc < TOC_BLOCKS; toc++){
        lecture_block(toc);
        for (int i = 0; i < FILE_PAR_TOC; i++) {
            if (buffer[i * sizeof(Fichier)] == 0x01) {
                quota++;
            }
        }
    }
    sprintf(str, "%d", quota);
    UART_puts(str);
    UART_pputs(" emplacements disponibles sur ");
    sprintf(str, "%d\n\r", MAX_FILE);
    UART_puts(str);
}

int main(void){
    UART_init();
    SPI_init(SPI_MASTER | SPI_FOSC_128 | SPI_MODE_0);

    _delay_ms(100);

    //Initialisation carte SD
    if(SD_init() != SD_SUCCESS){
        UART_pputs("Erreur Initialisation de la carte SD\r\n");
        return 0;
    }
    UART_pputs("Carte SD connectée\r\n");
    UART_pputs("Prêt à recevoir une commande\n\r");

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
        // Quotadisk
        else if(strcmp((char *)serial_buffer, "quotadisk") == 0){
            QUOTADISK();
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
                UART_pputs("Format de commande incorrect.\r\n");
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
    }
    return 0;
}
