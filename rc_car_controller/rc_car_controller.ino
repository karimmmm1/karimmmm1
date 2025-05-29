#include <Wire.h>
// #include <Adafruit_ICM20948.h> // Remplacé par Qmi8658c.h
#include <Qmi8658c.h> // Bibliothèque pour le capteur QMI8658C
#include <TFT_eSPI.h> // Requis pour l'écran AMOLED RM67162
#include <SPI.h>      // Souvent requis par TFT_eSPI
#include <ESP32Servo.h>

// Configuration de TFT_eSPI :
// La bibliothèque TFT_eSPI DOIT être configurée manuellement pour votre matériel.
// 1. Localisez le répertoire de la bibliothèque TFT_eSPI (généralement Documents/Arduino/libraries/TFT_eSPI).
// 2. Modifiez le fichier User_Setup_Select.h pour inclure un User_Setup.h spécifique,
//    OU modifiez directement un User_Setup.h existant (par exemple, User_Setup24_ST7789.h s'il est similaire et renommez-le).
//
// Pour l'écran RM67162 (QSPI) de la carte ESP32-S3-AMOLED-1.91-M, voici les définitions PROBABLES
// à inclure dans votre fichier de configuration User_Setup.h de TFT_eSPI :
// (Vérifiez ces broches avec la documentation officielle Spotpear si possible)
//
// #define RM67162_DRIVER
//
// #define TFT_WIDTH  240
// #define TFT_HEIGHT 536
//
// // Broches QSPI (hypothèse basée sur des modèles similaires, à vérifier)
// #define TFT_QSPI_CS   9  // Chip select
// #define TFT_QSPI_SCK  10 // Clock
// #define TFT_QSPI_D0   11 // Data 0 (MOSI)
// #define TFT_QSPI_D1   12 // Data 1 (MISO)
// #define TFT_QSPI_D2   13 // Data 2
// #define TFT_QSPI_D3   14 // Data 3
//
// #define TFT_QSPI_RST  21 // Reset
// #define TFT_QSPI_EN   42 // AMOLED Enable (pourrait être géré comme une broche BLK/backlight ou power enable)
// // Si TFT_QSPI_EN est utilisé pour l'alimentation, il faudra peut-être l'activer manuellement dans le code :
// // pinMode(42, OUTPUT);
// // digitalWrite(42, HIGH);
//
// // Assurez-vous qu'aucune autre configuration d'écran n'est active dans votre User_Setup.h.
// // Commentez les autres #define pour les drivers d'écran et les assignations de broches.
 
// Déclaration des objets
// Adafruit_ICM20948 icm; // Remplacé par qmi
Qmi8658c qmi(0x6B, 400000); // Adresse I2C 0x6B, fréquence 400kHz
Servo servoDroit;
Servo servoGauche;
TFT_eSPI tft = TFT_eSPI(); // Objet pour l'écran TFT_eSPI
 
// Pins des servos
const int pinServoDroit = 17; // Broche modifiée
const int pinServoGauche = 16; // Broche modifiée
 
// Variables de contrôle
float angleX, angleY, angleZ;
int angleServoDroit = 90; // Angle initial du servo droit (milieu)
int angleServoGauche = 90; // Angle initial du servo gauche (milieu)
 
// Déclaration des limites d'angle des servos
const int angleMax = 180;
const int angleMin = 0;
 
// Adresses I2C
// const uint8_t ICM20948_ADDR = 0x68; // Remplacé par l'adresse dans l'objet qmi
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
  Wire.begin(40, 39); // Initialize I2C with SDA on GP40, SCL on GP39
  delay(100); // Short delay after Wire.begin

  qmi8658_cfg_t config;
  config.qmi8658_mode = qmi8658_mode_acc_gyro; // Enable Accelerometer and Gyroscope
  config.acc_scale = acc_scale_4g;          // Accelerometer range +/- 4g
  config.acc_odr = acc_odr_250;             // Accelerometer output data rate 250Hz
  config.gyro_scale = gyro_scale_512dps;      // Gyroscope range +/- 512 dps
  config.gyro_odr = gyro_odr_250;            // Gyroscope output data rate 250Hz

  qmi8658_result_t result = qmi.open(&config);

  if (result == qmi8658_result_ok) {
    Serial.println("QMI8658 initialisé avec succès.");
    // You can optionally print deviceID and revisionID if available and needed:
    // Serial.print("QMI8658 DeviceID: 0x"); Serial.println(qmi.deviceID, HEX);
    // Serial.print("QMI8658 RevisionID: 0x"); Serial.println(qmi.deviceRevisionID, HEX);
  } else {
    Serial.print("Échec de l'initialisation du QMI8658, code : ");
    Serial.println(qmi.resultToString(result));
    while (true); // Loop indefinitely on failure
  }
}
 
void loop() {
  lireDonneesCapteur();
  afficherDonneesEcran(); // Nom de fonction mis à jour
  controlerServos();
  delay(100);
}
 
void lireDonneesCapteur() {
  qmi_data_t sensor_data;
  qmi.read(&sensor_data);

  angleX = sensor_data.acc_xyz.x;
  angleY = sensor_data.acc_xyz.y;
  angleZ = sensor_data.acc_xyz.z;

  // Optionally, you can also read gyroscope data if needed for other purposes
  // float gyroX = sensor_data.gyro_xyz.x;
  // float gyroY = sensor_data.gyro_xyz.y;
  // float gyroZ = sensor_data.gyro_xyz.z;
  // float temp = sensor_data.temperature;
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
