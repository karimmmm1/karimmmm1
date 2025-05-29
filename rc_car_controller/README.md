# Contrôleur de Voiture RC avec ESP32-S3 AMOLED (Français)

Ce projet est un microprogramme (firmware) pour une voiture télécommandée (RC) basée sur une carte de développement ESP32-S3 AMOLED 1.91 pouces de Spotpear. Il utilise un capteur QMI8658 (accéléromètre + gyroscope) pour capter les mouvements et contrôler des servomoteurs. L'affichage est géré par un écran AMOLED RM67162 via l'interface QSPI.

## Matériel Utilisé (Confirmé)

*   **Carte de développement**: Spotpear ESP32-S3 AMOLED 1.91 pouces (MCU: ESP32-S3R8). Modèle probable : ESP32-S3-Touch-AMOLED-1.91-M.
    *   Documentation du fabricant : [https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html](https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html) (Consultez ce lien pour les schémas de brochage exacts et les informations officielles).
*   **Capteur Inertiel (IMU)**: **QMI8658** (6-axes : accéléromètre 3 axes, gyroscope 3 axes) intégré à la carte.
    *   **Interface I2C** : SDA sur **GP40**, SCL sur **GP39**.
*   **Affichage**: Écran **AMOLED RM67162** de 1.91 pouces (résolution 240x536) intégré à la carte.
    *   **Interface** : **QSPI**.
*   **Servomoteurs**: Deux servos standards.
    *   Servo Gauche : Connecté à **GP16**.
    *   Servo Droit : Connecté à **GP17**.
*   **Batterie**: Appropriée pour alimenter l'ESP32 et les servos (connecteur MX1.25 sur la carte).

## Code et Modifications Apportées

Le code dans `rc_car_controller.ino` a été adapté pour :
*   Utiliser la bibliothèque `TFT_eSPI` pour l'affichage sur l'écran AMOLED RM67162.
*   Utiliser la bibliothèque `Qmi8658c` pour interagir avec le capteur de mouvement QMI8658.
*   Utiliser les broches GP16 et GP17 pour le contrôle des servomoteurs afin d'éviter les conflits avec les broches QSPI de l'écran.

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
        *   `Qmi8658c` par ALICHOUCHENE (pour le capteur IMU).
        *   `ESP32Servo` (par Kevin Harrington/John K. Bennett).
        *   `TFT_eSPI` (par Bodmer).
        *   `Wire` (généralement inclus avec le core ESP32).
5.  **Configurer la bibliothèque `TFT_eSPI`**:
    *   C'est une étape **cruciale** et **manuelle**.
    *   Localisez le répertoire de la bibliothèque `TFT_eSPI` (ex: `Documents/Arduino/libraries/TFT_eSPI`).
    *   Modifiez `User_Setup_Select.h` pour inclure un fichier de configuration `User_Setup.h` (ou modifiez un `User_Setup.h` existant).
    *   **Définitions à inclure dans votre `User_Setup.h` pour l'écran RM67162 (QSPI) de la carte ESP32-S3-AMOLED-1.91-M :**
        ```cpp
        #define RM67162_DRIVER

        #define TFT_WIDTH  240
        #define TFT_HEIGHT 536

        // Broches QSPI - HYPOTHÈSE BASÉE SUR DES MODÈLES SPOTPEAR SIMILAIRES
        // VÉrifiez IMPÉRATIVEMENT AVEC LA DOCUMENTATION OFFICIELLE DE VOTRE CARTE !
        #define TFT_QSPI_CS    9  // Chip select
        #define TFT_QSPI_SCK  10  // Clock
        #define TFT_QSPI_D0   11  // Data 0 (MOSI)
        #define TFT_QSPI_D1   12  // Data 1 (MISO)
        #define TFT_QSPI_D2   13  // Data 2
        #define TFT_QSPI_D3   14  // Data 3

        #define TFT_QSPI_RST  21  // Reset
        // #define TFT_QSPI_EN   42 // AMOLED Enable (ou TFT_BL si utilisé comme Backlight)
        // Si TFT_QSPI_EN (GPIO42) est utilisé pour l'alimentation de l'écran,
        // il faudra peut-être l'activer manuellement dans le setup() de votre code .ino :
        // pinMode(42, OUTPUT);
        // digitalWrite(42, HIGH);

        // Assurez-vous qu'aucune autre configuration d'écran ou de broches conflictuelles
        // n'est active dans votre User_Setup.h.
        ```
    *   **Note sur les broches QSPI** : Les broches listées ci-dessus (9-14, 21, 42) sont une **hypothèse** basée sur des cartes similaires. Il est **fortement recommandé** de vérifier ces broches sur la documentation officielle de Spotpear pour votre modèle exact. Une mauvaise configuration des broches empêchera l'écran de fonctionner.

## Compilation et Téléversement

*   Connectez la carte ESP32-S3 à votre ordinateur.
*   Sélectionnez le bon port COM (`Outils > Port`).
*   Cliquez sur "Téléverser".

## Logique de Contrôle des Servos

*   Le code utilise la lecture `angleY` de l'accéléromètre (provenant du QMI8658) pour contrôler les servos.
*   Si `angleY > 1` (ou `angleY < -1`), les servos bougent. Une accélération de 1g sur l'axe Y déclenchera le mouvement. Ce seuil pourrait nécessiter un ajustement en fonction de la sensibilité désirée.
*   Sinon, les servos sont en position neutre (90 degrés).
```
