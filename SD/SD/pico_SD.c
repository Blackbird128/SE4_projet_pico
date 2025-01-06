#include "pico_SD.h"




void SD_powerUpSeq()
{
    // make sure card is deselected
    CS_DISABLE();

    // give SD card time to power up
    _delay_ms(1);

    // send 80 clock cycles to synchronize
    for(uint8_t i = 0; i < 10; i++)
        SPI_transmit(0xFF);

    // deselect SD card
    CS_DISABLE();
    SPI_transmit(0xFF);
}


// Fonction pour envoyer une commande à la carte SD
uint8_t SD_send_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t response;

    // Sélectionner la carte SD
    PORTB &= ~(1 << SD_PIN);

    // Envoyer la commande
    SPI_transmit(0x40 | cmd);  // Le premier bit est toujours 1 pour les commandes
    SPI_transmit(arg >> 24);    // Argument haut
    SPI_transmit(arg >> 16);    // Argument milieu
    SPI_transmit(arg >> 8);     // Argument bas
    SPI_transmit(arg);          // Argument bas
    SPI_transmit(crc|0x01);         // CRC pour la commande

    // Attendre la réponse
    response = SPI_transmit(0xFF);  // Lire la réponse

    // Désélectionner la carte SD
    PORTB |= (1 << SD_PIN);

    return response;  // Retourner la réponse de la carte SD
}


uint8_t SD_readRes1()
{
    uint8_t i = 0, res1;

    // keep polling until actual data received
    while((res1 = SPI_transmit(0xFF)) == 0xFF)
    {
        i++;

        // if no data received for 8 bytes, break
        if(i > 8) break;
    }

    return res1;
}


uint8_t SD_goIdleState()
{
    // assert chip select
    SPI_transmit(0xFF);
    CS_ENABLE();
    SPI_transmit(0xFF);

    // send CMD0
    SD_send_command(0, 0, CMD_CRC);

    // read response
    uint8_t res1 = SD_readRes1();

    // deassert chip select
    SPI_transmit(0xFF);
    CS_DISABLE();
    SPI_transmit(0xFF);

    return res1;
}

// Fonction pour initialiser la carte SD
uint8_t SD_initialize() {
    // Réinitialiser le SPI
    SPI_init();

    // Mettre la carte SD dans un état de "non sélection" pendant 80 cycles
    PORTB |= (1 << SD_PIN);
    for (int i = 0; i < 10; i++) {
        SPI_transmit(0xFF); // envoyer 8x 0xFF pour réinitialiser la carte
    }

    // Envoyer la commande de réinitialisation de la carte SD
    if (SD_send_command(0, 0, CMD_CRC) != 0x01) {
        return 0; // La carte SD ne répond pas correctement
    }

    // Passer en mode SPI (CMD 1)
    if (SD_send_command(1, 0, CMD_CRC) != 0x00) {
        return 0; // La carte SD ne passe pas en mode SPI
    }

    // Si l'initialisation réussit, afficher un message de débogage
    printf("Carte SD initialisée avec succès.\n");

    return 1;  // La carte SD a été initialisée avec succès
}

// Fonction pour lire un secteur de la carte SD
uint8_t SD_read_sector(uint32_t sector, uint8_t* buffer) {
    // Envoyer la commande de lecture de secteur (CMD 17)
    if (SD_send_command(17, sector * 512, CMD_CRC) != 0x00) {
        return 0;  // La carte SD a échoué à répondre
    }

    // Lire les 512 octets du secteur
    for (int i = 0; i < 512; i++) {
        buffer[i] = SPI_transmit(0xFF);  // Lire un octet
    }

    // Vérifier la fin de la lecture du secteur (réponse 0xFF)
    SPI_transmit(0xFF); // Attendre la fin de la lecture

    return 1;  // Lecture réussie
}

