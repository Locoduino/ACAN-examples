/*
 * Exemple de liaison CAN pour la commande
 * 
 * emetteur.ino
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
 * Un objet pour le message CAN. Par defaut c'est un message
 * standard avec l'identifiant 0 et aucun octet de donnees
 */
CANMessage messageCAN;

/*
 * Les boutons poussoir sont relies aux broches 3 et 4 
 */
static const uint8_t brocheBouton1 = 3;
static const uint8_t brocheBouton2 = 4;

/*
 * Les anti rebond des boutons
 */
Bounce poussoir1;
Bounce poussoir2;

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
  pinMode(brocheBouton1, INPUT_PULLUP);
  poussoir1.attach(brocheBouton1);
  pinMode(brocheBouton2, INPUT_PULLUP);
  poussoir2.attach(brocheBouton2);
}



void loop()
{
  poussoir1.update();
  
  if (poussoir1.fell())
  {
    /* le bouton 1 a ete appuye, on envoie un message avec la donnee 1 */
    messageCAN.data[0] = 1; /* L'octet 0 des donnees a 1 */
    messageCAN.len = 1;     /* Un seul octet de donnees  */ 
    const bool ok = controleurCAN.tryToSend(messageCAN);
    if (ok) {
      Serial.println("message 1 envoye !");
    }
    else {
      Serial.println("echec de l'envoi de message 1");
    }
  }
  
  poussoir2.update();

  if (poussoir2.fell())
  {
    /* le bouton 2 a ete appuye, on envoie un message avec la donnee 2 */
    messageCAN.data[0] = 2; /* L'octet 0 des donnees a 1 */
    messageCAN.len = 1;     /* Un seul octet de donnees  */ 
    const bool ok = controleurCAN.tryToSend(messageCAN);
    if (ok) {
      Serial.println("message 2 envoye !");
    }
    else {
      Serial.println("echec de l'envoi de message 1");
    }
  }
}
