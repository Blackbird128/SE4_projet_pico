#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 256
#define MAX_FILENAME_LENGTH 16
#define BLOCKS_PER_FILE 2040
#define nom_vide "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"  // Chaîne remplie de 0x00


unsigned int CS[5];

void readBlock(unsigned int num, int offset, unsigned char *storage, int size) {
    FILE *file = fopen("filesystem.bin", "rb+"); // Utiliser "rb+" pour ouvrir le fichier en lecture et écriture
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }
    fseek(file, num * BLOCK_SIZE + offset, SEEK_SET);
    fread(storage, 1, size, file);
    fclose(file);
}

void writeBlock(unsigned int num, int offset, const unsigned char *storage, int size) {
    FILE *file = fopen("filesystem.bin", "rb+"); // Utiliser "rb+" pour ouvrir le fichier en lecture et écriture
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }
    fseek(file, num * BLOCK_SIZE + offset, SEEK_SET);
    fwrite(storage, 1, size, file);
    fclose(file);
}


int find_file(char* filename) {
    char data_block[BLOCK_SIZE];
    // Parcours tous les blocs du système de fichiers pour trouver un fichier correspondant
    for (int block_num = 0; block_num < BLOCKS_PER_FILE; block_num++) {
        // Lire le bloc de données
        readBlock(CS[1], block_num, data_block, sizeof(data_block));
        for (int i = 0; i < BLOCK_SIZE; i += MAX_FILENAME_LENGTH) {
            if (strncmp((char*)&data_block[i], filename, MAX_FILENAME_LENGTH) == 0) {
                // Fichier trouvé, retourner son adresse (bloc)
                return block_num * BLOCK_SIZE + i;
            }
        }
    }
    // Si le fichier n'a pas été trouvé
    return -1;
}


// Function to extend the filename with 0x00 until the length reaches MAX_FILENAME_LENGTH
char* extension_0x00(char* name, int length) {
    static char extended_name[MAX_FILENAME_LENGTH];
    int name_len = strlen(name);

    // Copy the original name into the extended_name buffer
    strncpy(extended_name, name, length);

    // If the name is shorter than the maximum length, fill the remaining space with 0x00
    if (name_len < length) {
        memset(extended_name + name_len, 0x00, length - name_len);
    }
    return extended_name;
}


int RENAME(char* nom_fichier_initial_1, char* nom_fichier_final_1)
{ // V1
    char data_block[BLOCK_SIZE];
    char nom_fichier_init[MAX_FILENAME_LENGTH];
    char nom_fichier_final[MAX_FILENAME_LENGTH];
    if (sizeof(nom_fichier_initial_1) != MAX_FILENAME_LENGTH) { //Mettre des 0x00 si la longueur n'est pas de MAX_FILENAME_LENGTH
        strcpy(nom_fichier_init, extension_0x00(nom_fichier_initial_1,MAX_FILENAME_LENGTH));
    }
    else
        strcpy(nom_fichier_init,nom_fichier_initial_1);
    if (sizeof(nom_fichier_final_1) != MAX_FILENAME_LENGTH){ //Mettre des 0x00 si la longueur n'est pas de MAX_FILENAME_LENGTH
        strcpy(nom_fichier_final, extension_0x00(nom_fichier_final_1, MAX_FILENAME_LENGTH));
    }
    else
        strcpy(nom_fichier_final,nom_fichier_final_1);
    int add_fichier = find_file(nom_fichier_init);
    if (add_fichier != -1) {
        int add_block = add_fichier / 16; // Adresse de block ou le nom est stocké
        int num_block = add_block * 16;
        int num_fichier_block = add_fichier - num_block; // Numéro de fichier dans le block de nom
        readBlock(CS[1], add_block, data_block, sizeof(data_block)); // Lecture du block ou le nom du fichier est stocké

        for (int i = 0; i < MAX_FILENAME_LENGTH; i++) {
            data_block[num_fichier_block + i] = nom_fichier_final[i]; // Changement du nom.
        }
        writeBlock(CS[1], add_block, data_block, sizeof(data_block)); // Réecriture du nouveau block.
        printf("\r\nRENAME DONE\r\n");
        return 1;
    } else {
        printf("\r\nFichier introuvable\r\n"); // Pas de fichier trouvé.
        return -1;
    }
}


void append(char* name, char* data, int len)
{
    char nom_fichier[MAX_FILENAME_LENGTH];
	// Etend le nom du fichier name avec des 0x00 et ce jusqu'à ce qu'il ai une longueur = MAX_FILENAME_LENGTH
    strcpy(nom_fichier,extension_0x00(name, MAX_FILENAME_LENGTH));
	// Renvoie l'adresse de l'emplacement du fichier s'il existe
    int addr_fichier = find_file(nom_fichier);
    int cmpt = 0;
    if (addr_fichier < 0) { // Fichier non existant : fichier à creer
        int addr_libre = find_file(nom_vide);  // Trouve un emplacement vide
        if (addr_libre < 0) {
            printf("Pas de place libre pour la création d'un fichier\r\n");
        } else {
            RENAME(nom_vide, nom_fichier); // Remplace le nom vide au bon emplacement par le nom du fichier
            int nb_block = (len / BLOCK_SIZE) + 1;
            if (nb_block > BLOCKS_PER_FILE) {
                printf("Longueur du fichier trop importante\r\n");
            }
            for (int i = addr_libre * BLOCKS_PER_FILE; i < addr_libre * BLOCKS_PER_FILE + nb_block; i++) {
                char data_block[BLOCK_SIZE];

                for (int j = 0; j < BLOCK_SIZE; j++) {
                    data_block[j] = data[cmpt * BLOCK_SIZE + j];
                }
                writeBlock(CS[1], i, data_block, sizeof(data_block));
                cmpt++;
            }
        }
    }
}

int main(){

    return 0;
}
