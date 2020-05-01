/*
 * Exemple de liaison CAN pour la commande
 * 
 * recepteur-led.ino
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
#include <Servo.h>

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

const int NB_LED = 2;

/*
 * Les LEDs sont connectees sur les broches 3 et 4
 */
const uint8_t brocheLED[NB_LED] = { 3, 4 };

void messagePourLesLEDs(const CANMessage & inMessage)
{
  Serial.print("Message LED reçu, changement de l'etat de la LED ");
  Serial.println(inMessage.data[0]);
  uint8_t numeroLED = inMessage.data[0];
  digitalWrite(brocheLED[numeroLED], !digitalRead(brocheLED[numeroLED]));
}

/* 
 * Définition des masques et filtres.
 * Il s'agit des messages d'identifiant 1, 
 * c'est à dire en binaire sur 11 bits (taille d'un identifiant
 * de trame standard) :
 * 00000000001
 * On ne veut que ces messages, donc le masque sélectionne
 * tous les bits :
 */
const ACAN2515Mask masque = standard2515Mask(0x7FF, 0, 0); /* Que des 1 sur 11 bits */

/* 
 *  Et on définit un filtre pour les messages relatifs aux LEDs.
 */
const ACAN2515AcceptanceFilter filtres[] = {
   { standard2515Filter(1, 0, 0), messagePourLesLEDs }
};

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
  /* 1 emplacement suffit dans la file d'attente de reception */
  reglages.mReceiveBufferSize = 1;
  /* Pas de file d'attente d'emission */
  reglages.mTransmitBuffer0Size = 0;
  /* Demarre le CAN */
  const uint16_t codeErreur = controleurCAN.begin(reglages, [] { controleurCAN.isr(); }, masque, filtres, 1 );
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
  for (uint8_t led = 0; led < NB_LED; led++) {
    pinMode(brocheLED[led], OUTPUT);
  }
}

void loop()
{
  controleurCAN.dispatchReceivedMessage();
}
