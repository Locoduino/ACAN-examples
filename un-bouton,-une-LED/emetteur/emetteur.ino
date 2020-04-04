/*
 * Exemple de liaison CAN pour la commande
 * 
 * emetteur.ino
 * 
 * Materiel :
 * - 1 carte CAN a base de MCP2515
 * - 1 arduino UNO
 * - 1 bouton poussoir
 * 
 * Bibliotheque employees (disponibles via le gestionnaire de bibliothèques) :
 * - acan2515
 * - Bounce2
 */

/*
 * Inclusions pour acan2515
 */
#include <ACANBuffer16.h>
#include <ACANBuffer.h>
#include <ACAN2515Settings.h>
#include <ACAN2515.h>
#include <CANMessage.h>

/*
 * Inclusion pour le bouton poussoir
 */
#include <Bounce2.h>

/*
 * Broches pour le chip select et l'interruption du MCP2515
 */
static const uint8_t MCP2515_CS  = 10;
static const uint8_t MCP2515_INT = 2;

/*
 * L'objet pour piloter le MCP2515. SPI designe l'objet
 * utilise pour la connexion SPI car sur certaines cartes
 * notamment les Teensy, il peut y avoir plusieurs SPI.
 */
ACAN2515 controleurCAN(MCP2515_CS, SPI, MCP2515_INT);

/*
 * La frequence du quartz du MCP2515 en hertz. 
 * Sur la carte LOCODUINO, il s'agit d'un quartz 16MHz
 * Sur les cartes CAN que l'on peut trouvez chez les revendeurs
 * chinois, il s'agit generalement d'un quartz 8MHz
 */
static const uint32_t QUARTZ_FREQUENCY = 16000000;

/*
 * Un objet pour le message CAN. Par defaut c'est un message
 * standard avec l'identifiant 0 et aucun octet de donnees
 */
CANMessage messageCAN;

/*
 * Le bouton poussoir est relie à la broche 3 
 */
static const uint8_t brocheBouton = 3;

/*
 * L'anti rebond du bouton
 */
Bounce poussoir;

void setup()
{
  /* Demarre la ligne serie */
  Serial.begin(115200);
  /* Demarre le SPI */
  SPI.begin();
  /* Configure le MCP2515 */
  Serial.println("Configuration du MCP2515");
  /* Fixe la vitesse du bus a 125 kbits/s */
  ACAN2515Settings reglages(QUARTZ_FREQUENCY, 125000);
  /* Demarre le CAN */
  const uint16_t codeErreur = controleurCAN.begin(reglages, [] { controleurCAN.isr(); } );
  /* Verifie que tout est ok */
  if (codeErreur == 0) {
    Serial.println("Configuration ok");
  }
  else {
    Serial.println("Probleme de connexion");
    while (1); 
  }

  /* Initialise le bouton */
  pinMode(brocheBouton, INPUT_PULLUP);
  poussoir.attach(brocheBouton);
}



void loop()
{
  poussoir.update();
  if (poussoir.fell())
  {
    /* le bouton a ete appuye, on envoie un message */
    const bool ok = controleurCAN.tryToSend(messageCAN);
    if (ok) {
      Serial.println("message envoye !");
    }
    else {
      Serial.println("echec de l'envoi");
    }
  }
}
