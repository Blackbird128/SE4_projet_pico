#define FILE_NAME 16
#define FIRST_FILE_BLOCK 2 // On garde les deux premiers blocs pour la TOC
#define BLOCK_PAR_FILE 4
#define MAX_FILE 8
#define BLOCK_SIZE 512

struct Fichier{
    uint8_t available;
    uint8_t starting_block;
    char name[FILE_NAME];
}typedef Fichier; //Taille 18 octets


void lecture_block(int block);

void ecriture_block(int block);

int fichier_existe(char *name);

int first_file_available();

void clear_buffer();

Fichier get_Fichier(char *name);

int void_get_index_from_TOC(Fichier fichier);

void FORMAT();

void LS();

void APPEND(char *name, uint8_t *data, int taille);

void READ(char *name);

void REMOVE(char *name);
