/*
 * Premier exemple de programme de l'article publé sur Locoduino:
 * https://www.locoduino.org/spip.php?article268
 */
#include <ACAN2515Settings.h> /* Pour les reglages  */
#include <ACAN2515.h>         /* Pour le controleur */
#include <CANMessage.h>       /* Pour les messages  */

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
static const uint32_t FREQUENCE_DU_QUARTZ = 8000000;

/*
 * La fréquence du bus CAN
 */
static const uint32_t FRENQUENCE_DU_BUS_CAN = 125000;

/*
 * Deux objets pour les messages CAN en emission et en reception.
 * Par defaut les messages sont standard avec l'identifiant 0
 * et aucun octet de donnees
 */
CANMessage messageCANEmission;
CANMessage messageCANReception;

void setup()
{
  /* Demarre la connexion serie */
  Serial.begin(115200);
  /* Demarre le SPI */
  SPI.begin();
  /* Configuration du MCP2515 */
  Serial.println("Configuration du MCP2515");
  ACAN2515Settings configuration(FREQUENCE_DU_QUARTZ, FRENQUENCE_DU_BUS_CAN);
  /* Loopback */
  configuration.mRequestedMode = ACAN2515Settings::LoopBackMode;
  /* Demarrage du CAN */
  const uint32_t codeErreur = controleurCAN.begin(configuration, [] { controleurCAN.isr(); });
  if (codeErreur != 0) {
    Serial.print("Erreur : 0x");
    Serial.println(codeErreur, HEX);
    while (1);
  }
  else {
    Serial.println("OK !");
  }
}

void loop()
{
  if (controleurCAN.receive(messageCANReception)) {
    Serial.println("recu !");
  }

  static uint32_t dateDenvoi = 0;
  static uint32_t compte = 0;

  uint32_t dateCourante = millis();
  if (dateCourante - dateDenvoi >= 2000) {
    if (controleurCAN.tryToSend(messageCANEmission)) {
      Serial.print("envoi ");
      Serial.println(compte++);
      dateDenvoi = dateCourante;
    }
  }
}
