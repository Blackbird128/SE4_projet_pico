#define FILE_NAME 14 //Passage à 14 caracteres pour que la struct fasse 16 octets (32*12=512)
#define FIRST_FILE_BLOCK 2 // On garde les deux premiers blocs pour la TOC
#define BLOCK_PAR_FILE 4
#define MAX_FILE 32 //La toc est pleine
#define BLOCK_SIZE 512

#define MAX_BUFFER 100 //Longueur max de la commande avec arguments lue en UART

struct Fichier{
    uint8_t available;
    uint8_t starting_block;
    char name[FILE_NAME];
}typedef Fichier; //Taille 16 octets


void lecture_block(uint32_t block);

void ecriture_block(uint32_t block);

int fichier_existe(char *name);

int first_file_available();

void clear_buffer();

Fichier get_Fichier(char *name);

int void_get_index_from_TOC(Fichier fichier);

void serial_read_line();

void print_block(int block);

void FORMAT();

void LS();

void APPEND(char *name, uint8_t *data, int taille);

void READ(char *name);

void REMOVE(char *name);

void RENAME(char *oldname, char *newname);

void COPY(char *source_name, char *dest_name);
