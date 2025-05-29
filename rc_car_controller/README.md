# Contrôleur de Voiture RC avec ESP32-S3 AMOLED et LVGL (Français)

Ce projet est un microprogramme (firmware) pour une voiture télécommandée (RC) basée sur une carte de développement ESP32-S3 AMOLED 1.91 pouces de Spotpear. Il utilise :
*   **LVGL (Light and Versatile Graphics Library)** pour la gestion de l'interface utilisateur sur l'écran.
*   **TFT_eSPI** comme pilote bas niveau pour l'écran AMOLED RM67162 (interface QSPI).
*   Un capteur **QMI8658** (accéléromètre + gyroscope) pour capter les mouvements.
*   Des **servomoteurs** pour le contrôle.

## Matériel Utilisé (Confirmé)

*   **Carte de développement**: Spotpear ESP32-S3 AMOLED 1.91 pouces (MCU: ESP32-S3R8). Modèle probable : ESP32-S3-Touch-AMOLED-1.91-M.
    *   Documentation du fabricant : [https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html](https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html) (Consultez ce lien pour les schémas de brochage exacts et les informations officielles).
*   **Capteur Inertiel (IMU)**: **QMI8658** (6-axes) intégré.
    *   **Interface I2C** : SDA sur **GP40**, SCL sur **GP39**.
*   **Affichage**: Écran **AMOLED RM67162** (1.91 pouces, 240x536 pixels) intégré.
    *   **Interface** : **QSPI**.
*   **Servomoteurs**: Deux servos standards.
    *   Servo Gauche : Connecté à **GP16**.
    *   Servo Droit : Connecté à **GP17**.

## Code et Modifications Apportées

Le code dans `rc_car_controller.ino` a été adapté pour :
*   Utiliser **LVGL** pour créer et gérer les éléments de l'interface utilisateur (labels pour afficher les données).
*   Utiliser la bibliothèque `TFT_eSPI` comme pilote d'affichage pour LVGL.
*   Utiliser la bibliothèque `Qmi8658c` pour interagir avec le capteur QMI8658.
*   Utiliser les broches GP16 et GP17 pour les servomoteurs.

## Configuration de l'Environnement de Développement (Arduino IDE)

1.  **Installer l'IDE Arduino**: Depuis [arduino.cc](https://www.arduino.cc/en/software).
2.  **Ajouter le support pour les cartes ESP32**:
    *   Dans `Fichier > Préférences > URL de gestionnaire de cartes supplémentaires`, ajoutez :
        ```
        https://espressif.github.io/arduino-esp32/package_esp32_index.json
        ```
    *   Dans `Outils > Type de carte > Gestionnaire de cartes...`, recherchez et installez "esp32 by Espressif Systems".
3.  **Sélectionner la carte et les paramètres**:
    *   `Outils > Type de carte` : "ESP32S3 Dev Module" (ou équivalent).
    *   `Outils > USB CDC On Boot` : `Enable` (pour la communication `Serial`).
4.  **Installer les bibliothèques Arduino nécessaires**:
    *   `Croquis > Inclure une bibliothèque > Gérer les bibliothèques...` :
        *   `lvgl` (recherchez "lvgl", installez la version officielle, souvent la plus récente v8 ou v9).
        *   `TFT_eSPI` par Bodmer.
        *   `Qmi8658c` par ALICHOUCHENE.
        *   `ESP32Servo` (par Kevin Harrington/John K. Bennett).
        *   `Wire` (généralement inclus).
        *   `SPI` (généralement inclus).

5.  **Configurer la bibliothèque `TFT_eSPI` (MANUEL)**:
    *   C'est une étape **cruciale**.
    *   Naviguez vers le dossier d'installation de `TFT_eSPI` (ex: `Documents/Arduino/libraries/TFT_eSPI`).
    *   Créez un dossier `User_Setups` s'il n'existe pas.
    *   Dans `User_Setups`, créez un fichier, par ex. `MonEcranSpotpear.h`.
    *   Copiez-y le contenu suivant (adaptez les broches QSPI si vous avez des informations plus précises de Spotpear) :
        ```cpp
        // Contenu pour User_Setups/MonEcranSpotpear.h
        #define RM67162_DRIVER

        #define TFT_WIDTH  240
        #define TFT_HEIGHT 536

        // Broches QSPI - HYPOTHÈSE BASÉE SUR DES MODÈLES SPOTPEAR SIMILAIRES
        // VÉrifiez IMPÉRATIVEMENT AVEC LA DOCUMENTATION OFFICIELLE DE VOTRE CARTE !
        #define TFT_QSPI_CS    9
        #define TFT_QSPI_SCK  10
        #define TFT_QSPI_D0   11 // MOSI
        #define TFT_QSPI_D1   12 // MISO
        #define TFT_QSPI_D2   13
        #define TFT_QSPI_D3   14

        #define TFT_QSPI_RST  21
        // #define TFT_QSPI_EN   42 // AMOLED Enable (ou TFT_BL)
        // Si GPIO42 est une broche d'activation, activez-la dans setup():
        // pinMode(42, OUTPUT); digitalWrite(42, HIGH);

        #define LOAD_GLCD
        #define LOAD_FONT2
        #define LOAD_FONT4
        #define LOAD_FONT6
        #define LOAD_FONT7
        #define LOAD_FONT8
        #define LOAD_GFXFF
        #define SMOOTH_FONT
        ```
    *   Ouvrez `User_Setup_Select.h` (à la racine de `TFT_eSPI`).
    *   Commentez toutes les autres lignes `#include <User_Setups/...>`.
    *   Ajoutez : `#include <User_Setups/MonEcranSpotpear.h>`

6.  **Configurer la bibliothèque `LVGL` (MANUEL)**:
    *   C'est aussi une étape **cruciale**.
    *   Naviguez vers le dossier d'installation de `lvgl` (ex: `Documents/Arduino/libraries/lvgl`).
    *   Trouvez `lv_conf_template.h`.
    *   Copiez-le et renommez la copie en `lv_conf.h` **dans le même répertoire**.
    *   Ouvrez `lv_conf.h` et modifiez/décommentez :
        ```c
        /* Config file for LVGL - Project specific configuration */
        #if 1 /*Set it to "1" to enable the content*/
        // ... (gardez le début du fichier)

        /* Maximal horizontal resolution to support by the library. */
        #define LV_HOR_RES_MAX (240)
        /* Maximal vertical resolution to support by the library. */
        #define LV_VER_RES_MAX (536)

        /* Color depth:
         * - 1:  1 byte per pixel (Grayscale)
         * - 8:  1 byte per pixel (Palette)
         * - 16: 2 bytes per pixel (RGB565)
         * - 32: 4 bytes per pixel (ARGB8888)
         */
        #define LV_COLOR_DEPTH 16

        /* Dot Per Inch: used to initialize default sizes.
         * E.g. a button with width = LV_DPI_DEF / 2 -> half inch on a 130 DPI screen */
        #define LV_DPI_DEF 130 /*[px/inch]*/

        /* Size of the memory available for LVGL's internal operating system. */
        // #define LV_MEM_SIZE (32U * 1024U) /*[bytes]*/ // Augmentez si nécessaire, surtout avec PSRAM

        /* Size of the buffer for drawing.
         * LVGL will render the graphics here first, then send the rendered image to the display.
         * The buffer size can be set freely but LVGL recommends setting it to at least 1/10 of the screen size.
         * For ESP32, if PSRAM is available, you can use a larger buffer in PSRAM.
         */
        #define LV_DISP_DRAW_BUF_SIZE (LV_HOR_RES_MAX * 10) // Buffer pour 10 lignes, modifiez si besoin

        /* Default font */
        #define LV_FONT_MONTSERRAT_14    1 // Activer la police Montserrat 14
        // #define LV_FONT_DEFAULT        &lv_font_montserrat_14 // Définir comme police par défaut

        /*---Needed Text Encoding Scheme---*/
        // #define LV_TXT_ENC LV_TXT_ENC_UTF8 // Usually default

        /*---Enable needed widgets---*/
        #define LV_USE_LABEL           1
        // #define LV_USE_BUTTON          1 // Si vous ajoutez des boutons plus tard

        /*---Others---*/
        /* Custom tick source */
        // #define LV_TICK_CUSTOM 0 // Utiliser le tick interne de LVGL, plus simple pour commencer

        // ... (gardez la fin du fichier)
        #endif /*End of "Content enable"*/
        ```
    *   Assurez-vous que le `#if 1` au début est bien activé.
    *   Pour `LV_FONT_DEFAULT`, si vous l'activez, assurez-vous que la police (ex: `lv_font_montserrat_14`) est bien activée aussi.

## Compilation et Téléversement

*   Après ces configurations manuelles, connectez la carte.
*   Sélectionnez le port COM et cliquez sur "Téléverser".

## Logique de Contrôle et Affichage

*   Le code utilise LVGL pour afficher les données du capteur QMI8658 et l'état des servos.
*   La fonction `loop()` appelle `lv_timer_handler()` pour les tâches LVGL.
*   Les seuils pour `controlerServos()` (basés sur `angleY`) pourraient nécessiter un ajustement car `angleY` est maintenant en 'g' (unité d'accélération).
```
