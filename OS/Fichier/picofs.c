#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHUNK_SIZE 64

#define BLOCK_SIZE 256
#define MAX_FILENAME_LENGTH 16
#define MAX_BLOCKS_PER_FILE 2040
#define MAX_FILES_IN_DIRECTORY 64
#define FILESYSTEM_SIZE (8 * 1024 * 1024) // 8 Mo
#define FIRST_DISPONIBILITY_CARD_BLOCK 1024
#define MAX_BLOCKS_PER_FILE_DESCRIPTION 16
#define FIRST_DATA_BLOCK 1040


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


void LS() {
    unsigned char buffer[MAX_FILENAME_LENGTH];
    int fileCount = 0;

    // Parcourir les blocs multiples de 16 à partir de 0 jusqu'à MAX_FILES_IN_DIRECTORY
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        readBlock(blockNum, 0, buffer, MAX_FILENAME_LENGTH);

        // Vérifier si le nom de fichier est vide
        if (buffer[0] != 0) {
            // Afficher le nom du fichier
            printf("%s\n", buffer);
            fileCount++;
        }
    }

    if (fileCount == 0) {
        printf("Aucun fichier trouvé.\n");
    }
}


void setBlockAvailability(int blockNum, int availability) {

    int byteIndexInCard = blockNum / 8; //Numéro d'octet dans la carte des blocs libres
    int blockIndexInCard = byteIndexInCard / 256; //Numéro du bloc dans la carte des blocs libres 
    int byteIndexInBlock = byteIndexInCard % 256; //Numéro d'octet dans le bloc contenant le bit de disponibilité
    int bitOffset = blockNum % 8; //Numéro du bit dans l'octet n°byteIndexInBlock du bloc blockIndexInCard
    
    //indice du bloc contenant le bit à mettre à jour dans le bloc de description
    int availabilityBlockNum = FIRST_DISPONIBILITY_CARD_BLOCK + blockIndexInCard;

    // Lire l'octet existant dans le bloc de disponibilité du super bloc
    unsigned char buffer;
    readBlock(availabilityBlockNum, byteIndexInBlock, &buffer, 1);

    // Mettre à jour le bit d'offset correspondant :
    if (availability==1) { // indisponible
        buffer |= (1 << bitOffset);  // Mettre le bit à 1
        
    } else { // disponible
        buffer &= ~(1 << bitOffset); // Mettre le bit à 0
    }

    // Écrire l'octet mis à jour dans le bloc de disponibilité du super bloc
    writeBlock(availabilityBlockNum, byteIndexInBlock, &buffer, 1);
}

int min(int a, int b) {
    return a < b ? a : b;
}

void RM(const char *filesystem_path, const char *filename) {
    int fileFound = -1;
    int offset;
    unsigned char fileBuffer[BLOCK_SIZE];
    
    // Parcourir les blocs réservés pour la description des fichiers (superbloc)
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        unsigned char filenameBuffer[MAX_FILENAME_LENGTH];
        readBlock(blockNum, 0, filenameBuffer, MAX_FILENAME_LENGTH);

        // Vérifier si le bloc contient le nom du fichier recherché
        if (strncmp((const char *)filenameBuffer, filename, MAX_FILENAME_LENGTH) == 0) {
            // Effacer le nom du fichier dans le superbloc
            memset(filenameBuffer, 0, MAX_FILENAME_LENGTH);
            writeBlock(blockNum, 0, filenameBuffer, MAX_FILENAME_LENGTH);
            fileFound = 1;
            offset = blockNum;
            break;
        }
    }

    // Fin de fonction si fichier inexistant
    if (fileFound == -1) {
        printf("Le fichier \"%s\" n'a pas été trouvé.\n", filename);
        return;
    }

    for (int blockNum = offset; blockNum < offset + 16; blockNum++) {
        int startOffset = 0;

        if (blockNum == offset) {
            startOffset = MAX_FILENAME_LENGTH;
        }

        for (int chunkStart = startOffset; chunkStart < BLOCK_SIZE; chunkStart += CHUNK_SIZE) {
            int chunkSize = min(CHUNK_SIZE, BLOCK_SIZE - chunkStart);
            readBlock(blockNum, chunkStart, fileBuffer, chunkSize);

            for (int i = 0; i < chunkSize; i += 2) {
                int blockNumData = (fileBuffer[i] << 8) | fileBuffer[i + 1];
                printf("blockNumData=%d\n",blockNumData);
                if (blockNumData == 0) {
                    writeBlock(blockNum, chunkStart, fileBuffer, chunkSize); 
                    printf("Le fichier \"%s\" a été supprimé avec succès.\n", filename);
                    return; // Sortir des boucles
                }
                setBlockAvailability(blockNumData, 0); // Marquer le bloc comme disponible
                fileBuffer[i] = 0;
                fileBuffer[i + 1] = 0;
            }

            writeBlock(blockNum, chunkStart, fileBuffer, chunkSize);
        }
    }

    printf("Le fichier \"%s\" a été supprimé avec succès.\n", filename);
}

// Fonction qui modifie le nom du fichier
void MV(const char *filesystem_path, const char *old_filename, const char *new_filename) {
    size_t sizeNew_filename = strlen(new_filename);
    size_t sizeOld_filename = strlen(old_filename);
    unsigned char filenameBuffer[MAX_FILENAME_LENGTH];
    int fileFound = 0;
    
    // Parcourir les blocs réservés pour la description des fichiers (superbloc)
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        
        readBlock(blockNum, 0, filenameBuffer, MAX_FILENAME_LENGTH);
        
        // Vérifier si le bloc contient le nom du fichier recherché
        if (strncmp((const char*)filenameBuffer, old_filename, MAX_FILENAME_LENGTH) == 0) {
            
            if (sizeNew_filename < MAX_FILENAME_LENGTH) {
                sizeNew_filename+=1;
            }
            
            // Écrire le nom du fichier dans l'emplacement
            writeBlock(blockNum, 0, (const unsigned char *)new_filename, sizeNew_filename);
            
            fileFound=1;

            break; // Nom modifié, sortir de la boucle
        }
    }
    if (fileFound == 1) {
        printf("Le nom du fichier \"%s\" a été renommé avec succès.\n", old_filename);
    } else {
        printf("Le fichier \"%s\" n'a pas été trouvé.\n", old_filename);
    }
}

// Renvoie le premier bloc de donné disponible
int findAvailableBlock() {
    unsigned char availabilityBuffer[BLOCK_SIZE];
    int offset;

    // Lire la carte de disponibilité (à partir du bloc 1)
    for (int blockNum = FIRST_DISPONIBILITY_CARD_BLOCK; blockNum < FIRST_DATA_BLOCK; blockNum++) {
        readBlock(blockNum, 0, availabilityBuffer, BLOCK_SIZE);
        
        if (blockNum==FIRST_DISPONIBILITY_CARD_BLOCK) {
            offset=FIRST_DATA_BLOCK/8;
        } else {
            offset=0;
        }

        // Parcourir les octets de la carte de disponibilité
        for (int byteIndex = offset; byteIndex < BLOCK_SIZE; byteIndex++) {
            unsigned char byte = availabilityBuffer[byteIndex];
            
            // Parcourir les bits de chaque octet
            for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
                // Vérifier si le bit est à 0 (bloc disponible)
                if ((byte & (1 << bitIndex)) == 0) {
                    // Calculer l'indice du bloc en fonction du bloc et du bit
                    int blockIndex = byteIndex * 8 + bitIndex;
                    return blockIndex; 
                }
            }
        }
    }

    // Aucun bloc disponible trouvé
    return -1;
}

// Fonction qui reconstitue le numéro de bloc à partir du tableau
int reconsituteNumber(unsigned char blockNumberBuffer[2]) {
    unsigned char octet1 = blockNumberBuffer[0];
    unsigned char octet2 = blockNumberBuffer[1];
    int dataBlockNum = (octet1 << 8) | octet2;
    return dataBlockNum;
}


// Fonction qui réparti un numéro de bloc sur deux octets
void createNumberBuffer(int dataBlockNum, unsigned char blockNumberBuffer[2]) {                    
    blockNumberBuffer[0] = (dataBlockNum >> 8) & 0xFF;
    blockNumberBuffer[1] = dataBlockNum & 0xFF;
}

void TYPE(const char *filesystem_path, const char *filename) {
    size_t sizeFilename = strlen(filename);
    unsigned char buffer[MAX_FILENAME_LENGTH];
    int placeFound = -1;
    int index_description_block = 0; // indice du bloc de descrption
    int block_counter = 1; //compteur de blocs utilisés
    int index_in_descrBlock = 0;
    int offset;
    int first_loop=1;
    
    if (sizeFilename > MAX_FILENAME_LENGTH) {
        printf("Impossible de créer le fichier, nom trop long\n");
        return;
    }
    
    // Parcours des blocs réservés pour la description des fichiers (superbloc)
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        readBlock(blockNum, 0, buffer, MAX_FILENAME_LENGTH);
        // Vérifier si le bloc est vide (pas de nom de fichier)
        if (buffer[0] == 0) {
            // Écrire le nom du fichier dans l'emplacement vide du superbloc
            if (sizeFilename < MAX_FILENAME_LENGTH) {
                sizeFilename += 1; // Ajouter '\0' s'il y a de la place
            }
            writeBlock(blockNum, 0, (const unsigned char *)filename, sizeFilename);
            placeFound = blockNum;
            
            // Lire les données depuis l'entrée standard et écrire dans le fichier
            int dataBlockNum = findAvailableBlock(); // Premier bloc de données à partir du bloc 1040
            int blockSizeUsed = 0; // Compteur d'octets dans le bloc actuel
            
            unsigned char chunkBuffer[CHUNK_SIZE];
            size_t bytesRead;

            while ((bytesRead = fread(chunkBuffer, 1, CHUNK_SIZE, stdin)) > 0) {
                
                // Écrire le chunk dans le bloc de données
                writeBlock(dataBlockNum, blockSizeUsed, chunkBuffer, bytesRead);
                setBlockAvailability(dataBlockNum, 1);
                
                // Écrire le numéro du bloc actuel dans la description du fichier
                unsigned char blockNumberBuffer[2];
                createNumberBuffer(dataBlockNum,blockNumberBuffer);
            
                blockSizeUsed += bytesRead;
                
                // Si le bloc actuel est plein, passer au bloc suivant OU si c'est le premier tour dans la boucle
                if (blockSizeUsed == BLOCK_SIZE  || first_loop == 1) {
                    
                    first_loop = -1;
                    
                    // Ecriture du numéro de bloc utilisé dans les blocs de description
                    if (index_description_block == 0) {
                        offset = MAX_FILENAME_LENGTH;
                    } else {
                        offset = 0;
                    }
                    writeBlock(placeFound + index_description_block, offset + index_in_descrBlock*2, blockNumberBuffer, 2);
                    index_in_descrBlock++;
                    
                    dataBlockNum = findAvailableBlock(); // Passer au bloc suivant
                    blockSizeUsed = 0; // Réinitialiser la taille utilisée
                    
                    // Passage au bloc de description suivant
                    if (block_counter == (BLOCK_SIZE/2-offset)) {
                        index_description_block++;
                        index_in_descrBlock=0;
                    }
                        
                    // Compteur de nombre de blocs utilisés
                    block_counter++;
                    
                    // Vérifie si on a atteint le nombre maximal de blocs par fichier
                    if (block_counter >= 2040) {
                        printf("Le fichier a atteint sa taille maximale\n");
                        return;
                    }
                }
            }
            
            break; // Fichier créé, sortir de la boucle
        }
    }
    
    if (placeFound != -1) {
        printf("Le fichier \"%s\" a été créé avec succès.\n", filename);
    } else {
        printf("Plus de place dans le système de fichier pour créer ce fichier.\n");
    }
}


void CAT(const char *filesystem_path, const char *filename) {
    unsigned char filenameBuffer[MAX_FILENAME_LENGTH];
    unsigned char byteBuffer[2];
    unsigned char dataBuffer[CHUNK_SIZE];
    int offset;
    
    // Parcours des blocs réservés pour la description des fichiers (superbloc)
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        readBlock(blockNum, 0, filenameBuffer, MAX_FILENAME_LENGTH);
        
        // Vérifier si le bloc contient le nom du fichier recherché
        if (strncmp((const char *)filenameBuffer, filename, MAX_FILENAME_LENGTH) == 0) {
            // Le nom du fichier a été trouvé
            
            // Parcours les blocs de description
            for (int descrBlockNum=0; descrBlockNum<MAX_BLOCKS_PER_FILE_DESCRIPTION; descrBlockNum++) {
                if (descrBlockNum==0) {
                    offset=MAX_FILENAME_LENGTH;
                } else {
                    offset=0;
                }
                
                // Lecture des octets deux par deux
                for (int i=0; i<BLOCK_SIZE;i+=2) {
                    readBlock(descrBlockNum+blockNum, offset+i, byteBuffer, 2);
                
                    // Lire les numéros de blocs associés à ce fichier depuis les blocs de description
                    int dataBlockNum = reconsituteNumber(byteBuffer);
                
                    // Vérifier si le numéro de bloc est valide (non nul)
                    if (dataBlockNum == 0) {
                        return; // Fin du fichier
                    }
                    
                    for (int chunkStart = 0; chunkStart < BLOCK_SIZE; chunkStart += CHUNK_SIZE) {
                        // Lire et afficher le contenu du bloc de données
                        readBlock(dataBlockNum, chunkStart, dataBuffer, CHUNK_SIZE);
                        fwrite(dataBuffer, 1, CHUNK_SIZE, stdout);
                    }
                }
            }
            
            return; // Fichier affiché, sortie de la fonction
        }
    } Encore plus
    
    // Si le fichier n'a pas été trouvé
    printf("Le fichier \"%s\" n'a pas été trouvé.\n", filename);
}


// Fonction pour copier un fichier existant du système de fichier
void CP(const char *filesystem_path, const char *source_filename, const char *destination_filename) {
    unsigned char source_filenameBuffer[MAX_FILENAME_LENGTH];
    unsigned char destination_filenameBuffer[MAX_FILENAME_LENGTH];
    unsigned char descriptionBuffer[CHUNK_SIZE];
    unsigned char numBuffer[2];
    int destination_offset;
    int source_offset;
    int offset;
    int numDataBlock;
    int newDataBlock;
    
    // Recherche du fichier source
    source_offset = -1;
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        readBlock(blockNum, 0, source_filenameBuffer, MAX_FILENAME_LENGTH);
        
        // Vérifier si le bloc contient le nom du fichier source
        if (strncmp((const char *)source_filenameBuffer, source_filename, MAX_FILENAME_LENGTH) == 0) {
            source_offset = blockNum;
            break;
        }
    }
    
    if (source_offset == -1) {
        printf("Le fichier source \"%s\" n'a pas été trouvé.\n", source_filename);
        return;
    }

    // Recherche d'un emplacement libre pour le fichier destination
    destination_offset = -1;
    for (int blockNum = 0; blockNum < MAX_FILES_IN_DIRECTORY; blockNum += 16) {
        readBlock(blockNum, 0, destination_filenameBuffer, MAX_FILENAME_LENGTH);
        
        // Vérifier si le bloc est vide (pas de nom de fichier)
        if (destination_filenameBuffer[0] == 0) {
            destination_offset = blockNum;
            break;
        }
    }
    
    if (destination_offset == -1) {
        printf("Plus de place dans le système de fichier pour créer la copie de \"%s\".\n", source_filename);
        return;
    }

    // Copie du nom de la copie dans le bloc de description
    writeBlock(destination_offset, 0, (const unsigned char *)destination_filename, strlen(destination_filename));

    // Copier les blocs de données associés au fichier source dans de nouveaux blocs de données pour le fichier destination
    for (int i = 0; i < MAX_BLOCKS_PER_FILE_DESCRIPTION; i++) {
        if (i == 0) { offset = MAX_FILENAME_LENGTH; } 
        else { offset = 0; }
        
        // Lecture des numéros de bloc dans les blocs de description
        for (int byteNum=0; byteNum<BLOCK_SIZE; byteNum+=2) {
            readBlock(source_offset + i, offset + byteNum, numBuffer, 2);
            

            numDataBlock = reconsituteNumber(numBuffer);
            if (numDataBlock == 0) {
                printf("La copie de \"%s\" sous le nom \"%s\" a été créée avec succès.\n", source_filename, destination_filename);
                return;
            }
            
            for (int chunkStart = 0; chunkStart < BLOCK_SIZE; chunkStart += CHUNK_SIZE) {
                // On stocke le bloc de données associé au fichier source dans descriptionBuffer
                readBlock(numDataBlock, chunkStart, descriptionBuffer, CHUNK_SIZE);
                
                if (chunkStart == 0) {
                    // Trouver un nouveau bloc de données disponible
                    newDataBlock = findAvailableBlock();
                    
                    // Mise à jour de la carte de disponibilités
                    setBlockAvailability(newDataBlock, 1);
                    
                    // Ecriture du numéro de bloc dans la description du fichier
                    createNumberBuffer(newDataBlock,numBuffer);
                    writeBlock(destination_offset+i, offset+byteNum, numBuffer, 2);
                }
            
                // Ecriture du bloc de données dans le premier bloc disponible
                writeBlock(newDataBlock, chunkStart, descriptionBuffer, CHUNK_SIZE);
            }
        }
    }
}



int main(int argc, char *argv[]) {
    // Vérification du nombre d'arguments
    if (argc < 3) {
        printf("Usage: %s filesystem_path ACTION [arguments...]\n", argv[0]);
        printf("Actions:\n");
        printf("  LS\n");
        printf("  TYPE filename\n");
        printf("  CAT filename\n");
        printf("  RM filename\n");
        printf("  MV old_filename new_filename\n");
        printf(" CP filename copy_filename\n");
        
        // setBlockAvailability(1040,1);
        // setBlockAvailability(1041,1);
        // setBlockAvailability(1042,1);
        // setBlockAvailability(1043,1);
        // printf("1er bloc dispo:%d\n",findAvailableBlock());
        return 1;
    }

    // Récupération des arguments
    const char *filesystem_path = argv[1];
    const char *action = argv[2];

    // Exécution de l'action correspondante en fonction des arguments passés
    if (strcmp(action, "LS") == 0) {
        LS(filesystem_path);
    } else if (strcmp(action, "TYPE") == 0) {
        if (argc < 4) {
            printf("Usage: %s filesystem_path TYPE filename\n", argv[0]);
            return 1;
        }
        TYPE(filesystem_path, argv[3]);
    } else if (strcmp(action, "CAT") == 0) {
        if (argc < 4) {
            printf("Usage: %s filesystem_path CAT filename\n", argv[0]);
            return 1;
        }
        CAT(filesystem_path, argv[3]);
    } else if (strcmp(action, "RM") == 0) { 
        if (argc < 4) {
            printf("Usage: %s filesystem_path RM filename\n", argv[0]);
            return 1;
        }
        RM(filesystem_path, argv[3]);
    } else if (strcmp(action, "MV") == 0) { 
        if (argc < 5) {
            printf("Usage: %s filesystem_path MV filename\n", argv[0]);
            return 1;
        }
        MV(filesystem_path, argv[3], argv[4]);
    } else if (strcmp(action, "CP") == 0) {
        if (argc < 5) {
            printf("Usage: %s filesystem_path MV filename\n", argv[0]);
            return 1;
        }
        CP(filesystem_path, argv[3], argv[4]);
        
    } else {
        printf("Action non reconnue. Actions possibles : LS, TYPE, CAT, RM, MV.\n");
        return 1;
    }

    return 0;
}

