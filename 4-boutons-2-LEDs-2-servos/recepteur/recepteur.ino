/*
 * Exemple de liaison CAN pour la commande
 * 
 * recepteur.ino
 * 
 * Materiel :
 * - 1 carte CAN a base de MCP2515
 * - 1 arduino UNO
 * - 2 LEDs et 2 servos
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

/*
 * Les servos sont connectés sur les broches 5 et 6
 */
const uint8_t brocheServo1 = 5;
const uint8_t brocheServo2 = 6;

Servo servo1;
Servo servo2;

void messagePourLesLEDs(const CANMessage & inMessage)
{
  Serial.print("Message LED reçu, changement de l'etat de la LED ");
  Serial.println(inMessage.data[0]);
  if (inMessage.data[0] == 1) {
    digitalWrite(brocheLED1, !digitalRead(brocheLED1));
  }
  else if (inMessage.data[0] == 2) {
    digitalWrite(brocheLED2, !digitalRead(brocheLED2));
  }
}

void messagePourLesServos(const CANMessage & inMessage)
{
  Serial.print("Message Servo reçu, changement de position du servo ");
  Serial.println(inMessage.data[0]);
  if (inMessage.data[0] == 1) {
    /* 
     * Les deux angles sont 45 et 135. Pour passer de 45 à 135,
     * on fait 180 - 45 = 135. Pour passer de 135 à 45, 
     * on fait 180 - 135 = 45
     */
    servo1.write(180 - servo1.read());
  }
  else if (inMessage.data[0] == 2) {
    servo2.write(180 - servo2.read());
  }

}

/* 
 * Définition des masques et filtres.
 * Il s'agit de deux messages d'identifiant 1 et 2, 
 * c'est à dire en binaire sur 11 bits (taille d'un identifiant
 * de trame standard) :
 * 00000000001
 * 00000000010
 * On ne veut que ces deux messages, donc le masque sélectionne
 * tous les bits :
 */
const ACAN2515Mask masque = standard2515Mask(0x7FF, 0, 0); /* Que des 1 sur 11 bits */
/* 
 *  Et on définit deux filtres, le premier pour les messages
 *  relatifs aux LEDs et le second pour les messages relatifs
 *  aux servos.
 */
const ACAN2515AcceptanceFilter filtres[] = {
   { standard2515Filter(1, 0, 0), messagePourLesLEDs }, 
   { standard2515Filter(2, 0, 0), messagePourLesServos }
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
  /* Demarre le CAN */
  const uint16_t codeErreur = controleurCAN.begin(reglages, [] { controleurCAN.isr(); }, masque, filtres, 2 );
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

  /*
   * Initialise les objets servo
   */
  servo1.attach(brocheServo1);
  servo2.attach(brocheServo2);
  servo1.write(45);
  servo2.write(45);
}

void loop()
{
  controleurCAN.dispatchReceivedMessage();
}
