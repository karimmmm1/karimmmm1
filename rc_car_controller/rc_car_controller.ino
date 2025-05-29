#include <Wire.h>
// #include <Adafruit_ICM20948.h> // Remplacé par Qmi8658c.h
#include <Qmi8658c.h> // Bibliothèque pour le capteur QMI8658C
#include <TFT_eSPI.h> // Requis pour l'écran AMOLED RM67162
#include <SPI.h>      // Souvent requis par TFT_eSPI
#include <ESP32Servo.h>
#include <lvgl.h>     // LVGL Graphics Library

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

// LVGL Display flushing
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

// LVGL Globals
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 536;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf_1[screenWidth * 10]; // Declare a buffer for 10 lines
// static lv_color_t buf_2[screenWidth * 10]; // Optional second buffer

// LVGL Label Objects for UI
lv_obj_t *label_accel_x;
lv_obj_t *label_accel_y;
lv_obj_t *label_accel_z;
lv_obj_t *label_servo_droit;
lv_obj_t *label_servo_gauche;
lv_obj_t *label_message; // For startup message
 
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

// LVGL Display flushing function
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}
 
// Initialise l'écran AMOLED avec TFT_eSPI et LVGL
void initialiseEcran() {
    // 1. Initialize TFT_eSPI
    tft.begin();
    tft.setRotation(0); // Ou votre rotation désirée
    // pinMode(42, OUTPUT); // Exemple pour AMOLED_EN si nécessaire (vérifiez le brochage de votre carte)
    // digitalWrite(42, HIGH);

    // 2. Initialize LVGL
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, screenWidth * 10); // Initialiser le buffer d'affichage

    // 3. Initialize LVGL Display Driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 4. Create LVGL UI Elements (Labels)
    lv_obj_t *scr = lv_scr_act(); // Obtenir l'écran actuel
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN); // Fond noir

    label_message = lv_label_create(scr);
    lv_label_set_text(label_message, "Voiture RC Demarrage...");
    lv_obj_set_style_text_color(label_message, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_message, LV_ALIGN_TOP_MID, 0, 10);

    label_accel_x = lv_label_create(scr);
    lv_label_set_text(label_accel_x, "Accel X: Attente...");
    lv_obj_set_style_text_color(label_accel_x, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_accel_x, LV_ALIGN_TOP_LEFT, 5, 40);

    label_accel_y = lv_label_create(scr);
    lv_label_set_text(label_accel_y, "Accel Y: Attente...");
    lv_obj_set_style_text_color(label_accel_y, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_accel_y, LV_ALIGN_TOP_LEFT, 5, 60);

    label_accel_z = lv_label_create(scr);
    lv_label_set_text(label_accel_z, "Accel Z: Attente...");
    lv_obj_set_style_text_color(label_accel_z, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_accel_z, LV_ALIGN_TOP_LEFT, 5, 80);
    
    label_servo_droit = lv_label_create(scr);
    lv_label_set_text(label_servo_droit, "Servo D: Attente...");
    lv_obj_set_style_text_color(label_servo_droit, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_servo_droit, LV_ALIGN_TOP_LEFT, 5, 100);

    label_servo_gauche = lv_label_create(scr);
    lv_label_set_text(label_servo_gauche, "Servo G: Attente...");
    lv_obj_set_style_text_color(label_servo_gauche, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_servo_gauche, LV_ALIGN_TOP_LEFT, 5, 120);
    
    Serial.println(F("Initialisation de l'écran et de LVGL terminée."));
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

// Nouvelle fonction pour mettre à jour les labels LVGL
void mettre_a_jour_donnees_lvgl() {
    lv_label_set_text_fmt(label_accel_x, "Accel X: %.2f", angleX);
    lv_label_set_text_fmt(label_accel_y, "Accel Y: %.2f", angleY);
    lv_label_set_text_fmt(label_accel_z, "Accel Z: %.2f", angleZ);
    lv_label_set_text_fmt(label_servo_droit, "Servo D: %d", angleServoDroit);
    lv_label_set_text_fmt(label_servo_gauche, "Servo G: %d", angleServoGauche);
}
 
void loop() {
    lv_timer_handler(); // Laisser LVGL gérer ses tâches
    delay(5); // Période de tick LVGL

    lireDonneesCapteur();
    // afficherDonneesEcran(); // Remplacé par LVGL
    mettre_a_jour_donnees_lvgl(); // Mettre à jour le contenu des labels LVGL
    controlerServos();
    
    // delay(100); // Le délai original, ajuster au besoin avec LVGL
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
 
// L'ancienne fonction afficherDonneesEcran() est maintenant supprimée car LVGL gère l'affichage.
 
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
