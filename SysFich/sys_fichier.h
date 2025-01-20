#define BLOCK_SIZE 512
#define FILE_NAME 14 //Passage à 14 caractères pour que la struct Fichier fasse 16 octets (32*16=512)
#define TOC_BLOCKS 2 // On garde les deux premiers blocs de la carte SD pour la TOC
#define BLOCK_PAR_FILE 4
#define FILE_PAR_TOC 32
#define MAX_FILE (TOC_BLOCKS * FILE_PAR_TOC) // deux toc de 32 fichiers donc 64 fichiers ici

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

int get_index_from_TOC(Fichier fichier);

void serial_read_line();

void print_block(int block);

void FORMAT();

void LS();

void APPEND(char *name, uint8_t *data, int taille);

void READ(char *name);

void REMOVE(char *name);

void RENAME(char *oldname, char *newname);

void COPY(char *source_name, char *dest_name);
