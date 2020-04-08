/*
 * Exemple de liaison CAN pour la commande
 * 
 * recepteur.ino
 * 
 * Materiel :
 * - 1 carte CAN a base de MCP2515
 * - 1 arduino UNO
 * - 2 LEDs
 * 
 * Bibliotheque employees (disponibles via le gestionnaire de bibliothèques) :
 * - acan2515
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
 * Les LEDs sont connectees sur les broches 3 et 4
 */
const uint8_t brocheLED1 = 3;
const uint8_t brocheLED2 = 4;

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
    Serial.println("Recepteur: configuration ok");
  }
  else {
    Serial.println("Recepteur: Probleme de connexion");
    while (1); 
  }

  /*
   * Initialise les broche des LEDs
   */
  pinMode(brocheLED1, OUTPUT);
  pinMode(brocheLED2, OUTPUT);
}

void loop()
{
  if (controleurCAN.available()) {
    /* Un message CAN est arrive */
    static uint32_t numero = 0;
    /* 
     * On lit le message.
     */
    controleurCAN.receive(messageCAN) ;
    Serial.print("Recepteur: ");
    Serial.print(++numero);
    Serial.print(" message recu : ") ;
    Serial.println(messageCAN.data[0]);
    /* l'etat de la LED est change */
    if (messageCAN.data[0] == 1) {
      digitalWrite(brocheLED1, !digitalRead(brocheLED1));
    }
    else if (messageCAN.data[0] == 2) {
      digitalWrite(brocheLED2, !digitalRead(brocheLED2));
    }
  }
}
