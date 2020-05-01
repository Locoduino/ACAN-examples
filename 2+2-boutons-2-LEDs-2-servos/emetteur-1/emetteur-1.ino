/*
 * Exemple de liaison CAN pour la commande
 * 
 * emetteur-1.ino
 * 
 * Materiel :
 * - 1 carte CAN a base de MCP2515
 * - 1 arduino UNO
 * - 2 boutons poussoir
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
 * Sur les cartes CAN que l'on peut trouvez chez les revendeurs
 * chinois, il s'agit generalement d'un quartz 8MHz
 */
static const uint32_t FREQUENCE_DU_QUARTZ = 8ul * 1000ul * 1000ul;

/*
 * La fréquence du bus CAN
 */
static const uint32_t FREQUENCE_DU_BUS_CAN = 125ul * 1000ul;

/*
 * Un objet pour le message CAN des LEDs. Par defaut c'est un message
 * standard avec l'identifiant 0 et aucun octet de donnees
 */
CANMessage messageCANLEDs;

/*
 * Un objet pour le message CAN des Servos. Par defaut c'est un message
 * standard avec l'identifiant 0 et aucun octet de donnees
 */
CANMessage messageCANServos;

/*
 * Les boutons poussoir sont relies aux broches 3 à 6 
 */
static const uint8_t brocheBouton[] = { 3, 4 };

/*
 * Les anti rebond des boutons
 */
Bounce poussoir[2];

void setup()
{
  /* Demarre la ligne serie */
  Serial.begin(115200);
  /* Demarre le SPI */
  SPI.begin();
  /* Configure le MCP2515 */
  Serial.println("Configuration du MCP2515");
  /* Fixe la vitesse du bus a 125 kbits/s */
  ACAN2515Settings reglages(FREQUENCE_DU_QUARTZ, FREQUENCE_DU_BUS_CAN);
  /* Pas de file d'attente de reception */
  reglages.mReceiveBufferSize = 0;
  /* 1 emplacement suffit dans la file d'attente d'emission */
  reglages.mTransmitBuffer0Size = 1;
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

  /* Initialise les boutons */
  for (uint8_t bouton = 0; bouton < 2; bouton++) {
    pinMode(brocheBouton[bouton], INPUT_PULLUP);
    poussoir[bouton].attach(brocheBouton[bouton]);
  }
  /* 
   * Initialise les messages. Le message pour les LEDs a un identifiant à 1
   * Celui des servos a un identifiant à 2.
   */
  messageCANLEDs.id = 1;

  messageCANServos.id = 2;
}



void loop()
{
  poussoir[0].update();
  poussoir[1].update();

  if (poussoir[0].fell()) {
    const bool ok = controleurCAN.tryToSend(messageCANLEDs);
    if (ok) {
      Serial.println("led envoye !");
    }
    else {
      Serial.println("echec de l'envoi de la led");
    }
  }

  if (poussoir[1].fell()) {
    const bool ok = controleurCAN.tryToSend(messageCANServos);
    if (ok) {
      Serial.println("servo envoye !");
    }
    else {
      Serial.println("echec de l'envoi du servo");
    }
  }  
}
