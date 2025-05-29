# Contrôleur de Voiture RC avec ESP32-S3 AMOLED (Français)

Ce projet est un microprogramme (firmware) pour une voiture télécommandée (RC) basée sur une carte de développement ESP32-S3 avec un écran AMOLED. Il utilise un capteur QMI8658 (accéléromètre + gyroscope) pour capter les mouvements et contrôler des servomoteurs.

## Matériel Utilisé (Confirmé)

*   **Carte de développement**: Spotpear ESP32-S3 AMOLED 1.91 pouces (MCU: ESP32-S3R8)
    *   Documentation probable : [https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html](https://spotpear.com/wiki/ESP32-S3-1.91-inch-AMOLED-Display-TouchScreen.html) (Consultez ce lien pour les schémas de brochage exacts)
*   **Capteur Inertiel (IMU)**: **QMI8658** (6-axes : accéléromètre 3 axes, gyroscope 3 axes) intégré à la carte.
*   **Affichage**: Écran **AMOLED RM67162** de 1.91 pouces (résolution 240x536) intégré à la carte, interface **QSPI**.
*   **Servomoteurs**: Deux servos standards pour la direction et/ou la propulsion.
*   **Batterie**: Appropriée pour alimenter l'ESP32 et les servos (connecteur MX1.25 sur la carte).

## Code et Modifications Nécessaires

Le code actuel dans `rc_car_controller.ino` est un point de départ qui a été adapté pour utiliser `TFT_eSPI` pour l'affichage. **Cependant, il est toujours basé sur l'utilisation d'un capteur ICM20948, ce qui est incorrect pour cette carte.**

**Modifications impératives pour la carte Spotpear ESP32-S3 AMOLED 1.91 pouces avec QMI8658 :**

*   **Bibliothèque du Capteur IMU**:
    *   Retirer l'utilisation de `Adafruit_ICM20948.h`.
    *   Trouver et intégrer une bibliothèque Arduino compatible avec le capteur **QMI8658**. Des pistes incluent la `SensorLib` (mentionnée dans la documentation d'autres cartes Spotpear/Waveshare pour le QMI8658) ou des bibliothèques open-source pour QMI8658 disponibles sur GitHub.
    *   Adapter les fonctions `initialiseCapteur()` et `lireDonneesCapteur()` pour utiliser la nouvelle bibliothèque du QMI8658.
*   **Brochage (Pinout)**:
    *   Consulter impérativement la documentation de la carte (lien ci-dessus) pour identifier les **broches I2C exactes (SDA, SCL)** utilisées par le capteur QMI8658. L'initialisation I2C (`Wire.begin(SDA_PIN, SCL_PIN)`) devra utiliser ces broches si elles ne sont pas les broches par défaut de l'ESP32-S3.
    *   Identifier les **broches QSPI exactes** utilisées par l'écran RM67162 pour configurer correctement la bibliothèque `TFT_eSPI`.
*   **Affichage**: La bibliothèque `TFT_eSPI` est le bon choix (confirmé par Spotpear pour le RM67162). La configuration de `TFT_eSPI` (voir ci-dessous) est cruciale.

## Configuration de l'Environnement de Développement (Arduino IDE)

1.  **Installer l'IDE Arduino**: Téléchargez et installez la dernière version depuis [arduino.cc](https://www.arduino.cc/en/software).
2.  **Ajouter le support pour les cartes ESP32**:
    *   Ouvrez l'IDE Arduino.
    *   Allez dans `Fichier > Préférences`.
    *   Dans le champ "URL de gestionnaire de cartes supplémentaires", ajoutez l'URL suivante :
        ```
        https://espressif.github.io/arduino-esp32/package_esp32_index.json
        ```
    *   Cliquez sur `OK`.
    *   Allez dans `Outils > Type de carte > Gestionnaire de cartes...`.
    *   Recherchez "esp32" et installez le paquet "esp32 by Espressif Systems" (version 3.0.0 ou ultérieure recommandée).
3.  **Sélectionner la carte et les paramètres**:
    *   Allez dans `Outils > Type de carte` et cherchez une carte appropriée comme "ESP32S3 Dev Module".
    *   **Important**: Pour la communication série via le port USB natif de l'ESP32-S3, allez dans `Outils > USB CDC On Boot` et sélectionnez `Enable`.
    *   Configurez les autres paramètres de la carte (Schéma de partition, etc.) si nécessaire.
4.  **Installer les bibliothèques nécessaires**:
    *   Allez dans `Croquis > Inclure une bibliothèque > Gérer les bibliothèques...`.
    *   Installez les bibliothèques suivantes :
        *   **Pour le capteur QMI8658**: Recherchez une bibliothèque compatible (par exemple, `SensorLib` si disponible dans le gestionnaire, ou d'autres comme celles pour "QMI8658" trouvées sur GitHub à installer manuellement via .zip).
        *   `ESP32Servo` (par Kevin Harrington/John K. Bennett)
        *   `TFT_eSPI` (par Bodmer)

5.  **Configurer TFT_eSPI**:
    *   Consultez la documentation de votre carte Spotpear ESP32-S3 AMOLED 1.91 pouces pour identifier les **broches QSPI exactes** utilisées par l'écran RM67162.
    *   Localisez le répertoire de la bibliothèque `TFT_eSPI` dans votre dossier de bibliothèques Arduino (généralement `Documents/Arduino/libraries/TFT_eSPI`).
    *   Modifiez le fichier `User_Setup_Select.h` pour décommenter l'inclusion d'un fichier de configuration adapté ou, plus probablement, créez/modifiez un `User_Setup.h` spécifique.
    *   Vous devrez y définir :
        *   Le pilote : `#define RM67162_DRIVER`
        *   La résolution : `#define TFT_WIDTH  240`, `#define TFT_HEIGHT 536`
        *   Les broches QSPI (par exemple `#define TFT_MOSI PIN_QSPI_D0`, `#define TFT_SCLK PIN_QSPI_CLK`, etc., en utilisant les numéros de broche GPIO corrects pour votre carte). **Ces broches sont absolument nécessaires.**
        *   L'interface QSPI.
    *   Des exemples de configurations pour écrans QSPI avec ESP32-S3 sont disponibles dans la bibliothèque `TFT_eSPI` (souvent dans le dossier `User_Setups`).

## Compilation et Téléversement

*   Connectez la carte ESP32-S3 à votre ordinateur via USB.
*   Dans l'IDE Arduino, sélectionnez le bon port COM dans `Outils > Port`.
*   Cliquez sur le bouton "Téléverser" (flèche droite).

## Contrôle des Servos

Le code actuel ajuste la position des servos en fonction de la lecture de `angleY` de l'accéléromètre (qui devra provenir du QMI8658).
*   Si `angleY > 1`, les servos tournent dans des directions opposées.
*   Si `angleY < -1`, les servos tournent dans les directions opposées.
*   Sinon, les servos sont positionnés à 90 degrés (position neutre).

Les broches des servos sont définies par `pinServoDroit` (GPIO13) et `pinServoGauche` (GPIO12). Vérifiez sur le schéma de votre carte que ces broches sont disponibles et non conflictuelles.
```
