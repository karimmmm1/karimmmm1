#include <Wire.h>
#include <Adafruit_ICM20948.h>
#include <TFT_eSPI.h> // Requis pour l'écran AMOLED RM67162
#include <SPI.h>      // Souvent requis par TFT_eSPI
#include <ESP32Servo.h>

// Configuration de TFT_eSPI :
// Assurez-vous que le fichier User_Setup_Select.h dans la bibliothèque TFT_eSPI
// est configuré pour utiliser le driver RM67162 et les bonnes broches QSPI
// pour votre ESP32-S3. Par exemple:
// #define RM67162_DRIVER
// #define TFT_WIDTH  240
// #define TFT_HEIGHT 536
// Définissez les broches QSPI (CS, SCK, MOSI, MISO, etc.) spécifiques à votre carte.
 
// Déclaration des objets
Adafruit_ICM20948 icm;
Servo servoDroit;
Servo servoGauche;
TFT_eSPI tft = TFT_eSPI(); // Objet pour l'écran TFT_eSPI
 
// Pins des servos
const int pinServoDroit = 13;
const int pinServoGauche = 12;
 
// Variables de contrôle
float angleX, angleY, angleZ;
int angleServoDroit = 90; // Angle initial du servo droit (milieu)
int angleServoGauche = 90; // Angle initial du servo gauche (milieu)
 
// Déclaration des limites d'angle des servos
const int angleMax = 180;
const int angleMin = 0;
 
// Adresses I2C
const uint8_t ICM20948_ADDR = 0x68;
// const uint8_t OLED_ADDR = 0x3C; // Plus nécessaire avec TFT_eSPI via QSPI
 
void setup() {
  Serial.begin(115200);
  initialiseServos();
  initialiseEcran(); // Nom de fonction mis à jour
  initialiseCapteur();
}
 
void initialiseServos() {
  servoDroit.attach(pinServoDroit);
  servoGauche.attach(pinServoGauche);
  servoDroit.write(angleServoDroit); // Position initiale
  servoGauche.write(angleServoGauche); // Position initiale
}
 
// Initialise l'écran AMOLED avec TFT_eSPI
void initialiseEcran() {
  tft.init(); // Initialisation de l'écran (tft.begin() peut aussi être utilisé)
  tft.fillScreen(TFT_BLACK); // Efface l'écran avec du noir
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Texte blanc sur fond noir
  tft.setCursor(0, 0);
  tft.println("Voiture RC Demarrage...");
  // display.display(); // Plus nécessaire avec TFT_eSPI pour le dessin direct
  delay(2000);
  // Note: La vérification de l'échec d'initialisation avec TFT_eSPI
  // est moins standardisée que pour Adafruit_SSD1306.
  // Souvent, si tft.init() échoue, le code bloquera ou l'écran restera noir.
  // Vous pouvez ajouter des vérifications spécifiques si votre matériel/setup le supporte.
  Serial.println(F("Initialisation de l'écran TFT_eSPI terminée (pas de retour d'erreur standard)."));
}
 
void initialiseCapteur() {
  if (!icm.begin_I2C(ICM20948_ADDR)) {
    Serial.println(F("Échec de l'initialisation du capteur ICM20948"));
    while (true);
  }
  Serial.println("ICM20948 initialisé avec succès.");
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);
  icm.setAccelRateDivisor(10);
  icm.setGyroRateDivisor(10);
 
  // Activer les filtres passe-bas numériques avec les paramètres appropriés
  icm.enableAccelDLPF(true, ICM20X_ACCEL_FREQ_5_7_HZ);  // Active le DLPF de l'accéléromètre
  icm.enableGyroDLPF(true, ICM20X_GYRO_FREQ_5_7_HZ);    // Active le DLPF du gyroscope
}
 
void loop() {
  lireDonneesCapteur();
  afficherDonneesEcran(); // Nom de fonction mis à jour
  controlerServos();
  delay(100);
}
 
void lireDonneesCapteur() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp);
  angleX = accel.acceleration.x;
  angleY = accel.acceleration.y;
  angleZ = accel.acceleration.z;
}
 
// Affiche les données du capteur sur l'écran AMOLED
void afficherDonneesEcran() {
  tft.fillScreen(TFT_BLACK); // Efface l'écran (ou utilisez setTextColor avec fond pour éviter le clignotement)
  tft.setCursor(0, 0);
  tft.print("Accel X: "); tft.println(angleX);
  tft.print("Accel Y: "); tft.println(angleY);
  tft.print("Accel Z: "); tft.println(angleZ);
  // display.display(); // Plus nécessaire avec TFT_eSPI
}
 
void controlerServos() {
  if (angleY > 1) {
    angleServoDroit = constrain(angleServoDroit + 5, angleMin, angleMax);
    angleServoGauche = constrain(angleServoGauche - 5, angleMin, angleMax);
  } else if (angleY < -1) {
    angleServoDroit = constrain(angleServoDroit - 5, angleMin, angleMax);
    angleServoGauche = constrain(angleServoGauche + 5, angleMin, angleMax);
  } else {
    angleServoDroit = 90;
    angleServoGauche = 90;
  }
  servoDroit.write(angleServoDroit);
  servoGauche.write(angleServoGauche);
  Serial.print("Servo Droit: "); Serial.print(angleServoDroit);
  Serial.print(" | Servo Gauche: "); Serial.println(angleServoGauche);
}
