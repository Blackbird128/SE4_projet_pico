#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "pico_SD.h"
#include "pico_serial.h"

// Définitions des pins pour SPI
#define CS_PIN    10
#define MOSI_PIN  11
#define MISO_PIN  12
#define SCK_PIN   13
#define PD0 0
#define SPI_SPEED 4 // Définir la vitesse SPI, plus bas si vous avez des problèmes de fiabilité.

// Fonction pour initialiser SPI
void SPI_init() {
    // Configurer MOSI, SCK comme sorties, MISO comme entrée
    DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN);
    DDRB &= ~(1 << MISO_PIN);

    // Activer SPI, mode maître, clock = F_CPU / 16
    SPCR = (1 << SPE) | (1 << MSTR) | (SPI_SPEED << SPR0);
}

// Fonction pour envoyer un octet via SPI
uint8_t SPI_transmit(uint8_t data) {
    SPDR = data; // Mettre les données dans le registre de données SPI
    while (!(SPSR & (1 << SPIF))) { // Attendre la fin de la transmission
        ;
    }
    return SPDR; // Retourner l'octet reçu
}

// Fonction pour envoyer une commande à la carte SD
uint8_t SD_send_command(uint8_t cmd, uint32_t arg) {
    uint8_t response;

    // Sélectionner la carte SD
    PORTB &= ~(1 << CS_PIN);

    // Envoyer la commande
    SPI_transmit(0x40 | cmd);  // Le premier bit est toujours 1 pour les commandes
    SPI_transmit(arg >> 24);    // Argument haut
    SPI_transmit(arg >> 16);    // Argument milieu
    SPI_transmit(arg >> 8);     // Argument bas
    SPI_transmit(arg);          // Argument bas
    SPI_transmit(0x95);         // CRC pour la commande

    // Attendre la réponse
    response = SPI_transmit(0xFF);  // Lire la réponse

    // Désélectionner la carte SD
    PORTB |= (1 << CS_PIN);

    return response;  // Retourner la réponse de la carte SD
}

// Fonction pour initialiser la carte SD
uint8_t SD_initialize() {
    // Réinitialiser le SPI
    SPI_init();

    // Mettre la carte SD dans un état de "non sélection" pendant 80 cycles
    PORTB |= (1 << CS_PIN);
    for (int i = 0; i < 10; i++) {
        SPI_transmit(0xFF); // envoyer 8x 0xFF pour réinitialiser la carte
    }

    // Envoyer la commande de réinitialisation de la carte SD
    if (SD_send_command(0, 0) != 0x01) {
        return 0; // La carte SD ne répond pas correctement
    }

    // Passer en mode SPI (CMD 1)
    if (SD_send_command(1, 0) != 0x00) {
        return 0; // La carte SD ne passe pas en mode SPI
    }

    // Si l'initialisation réussit, afficher un message de débogage
    printf("Carte SD initialisée avec succès.\n");

    return 1;  // La carte SD a été initialisée avec succès
}

// Fonction pour lire un secteur de la carte SD
uint8_t SD_read_sector(uint32_t sector, uint8_t* buffer) {
    // Envoyer la commande de lecture de secteur (CMD 17)
    if (SD_send_command(17, sector * 512) != 0x00) {
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

int main(void) {
    //DDRB |= (1<<PD0);

    SPI_init;
    uint8_t buffer[512];  // Buffer pour stocker un secteur de la carte SD
    uint32_t sector = 0;  // Numéro de secteur à lire (ici, le premier secteur)

    /*// Initialiser le port SPI
    if (!SD_initialize()) {
        printf("Erreur d'initialisation de la carte SD.\n");
        return 1;  // Arrêter le programme si l'initialisation échoue
    }

   // SPI_transmit(0x01);
   // PORTD &= (0<<PD0);
    // Lire un secteur de la carte SD
    printf("Lecture du secteur %lu...\n", sector); // Débogage
    if (SD_read_sector(sector, buffer)) {
        printf("Lecture réussie du secteur %lu.\n", sector);

        // Afficher les premiers octets du secteur
        for (int i = 0; i < 64; i++) {
            printf("0x%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
    } else {
        printf("Erreur de lecture du secteur %lu.\n", sector);
    }*/

    return 0;
}
