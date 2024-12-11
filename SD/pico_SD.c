#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "pico_SD.h"
#include "../pico_SPI.h"


// Définitions des pins pour SPI
#define CS_PIN    10
#define MOSI_PIN  11
#define MISO_PIN  12
#define SCK_PIN   13
#define PD0 0
#define SPI_SPEED 4 // Définir la vitesse SPI, plus bas si vous avez des problèmes de fiabilité.


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

