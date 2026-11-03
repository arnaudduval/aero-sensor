# Firmware

## État

Seul existe `firmware/legacy_cda_logger/cda_logger.ino`, écrit avant plusieurs décisions. Il est à considérer comme référence de départ, pas comme base à garder telle quelle :

- Cible ESP32 classique (SDA 21, SCL 22, 100 kHz) sur un seul bus ; l'architecture retenue est le DevKit V1 avec deux bus I2C (21/22 et 25/26).
- Log CSV sur carte SD (`millis_ms,dp_dyn_pa,dp_yaw_pa,power_w,speed_kmh`, 20 Hz, SD_CS_PIN = 5) ; l'architecture retenue est LittleFS binaire, sans SD.
- Mesure déclenchée (commande 0x372D) avec `delay(10)` et clock stretching ; à remplacer par la mesure continue.
- Deux capteurs sur un seul bus aux adresses 0x25 et 0x26 (SDP810 + SDP811) ; décidé depuis : 2 × SDP810 (0x25), un par bus.
- BLE : NimBLE-Arduino en central, se connecte au premier appareil annonçant 0x1818 puis 0x1816 ; WHEEL_CIRCUMFERENCE_MM = 2105 (placeholder à mesurer).
- Ce qu'il fait déjà bien : CRC-8 Sensirion (poly 0x31, init 0xFF), lecture 9 octets (pression, température, facteur d'échelle), callbacks de notification (puissance aux octets [2:3] du Cycling Power Measurement ; vitesse roue à partir des tours cumulés et de l'horodatage en 1/1024 s du CSC).
- Il n'a jamais été compilé ni testé sur matériel.

## Spécification du firmware cible (ESP32 DevKit V1, classique, 4 Mo)

1. Matériel : ESP32 DevKit V1 (ESP32 classique, flash 4 Mo, micro-USB avec pont USB-série). Deux bus I2C : `Wire.begin(21, 22)` (bus A, SDP810 n°1) et `Wire1.begin(25, 26)` (bus B, SDP810 n°2 + OLED) ; une instance de lecture par bus, adresse 0x25 sur les deux. Bus et adresses restent configurables.
2. Mesure : mesure continue de pression différentielle obligatoire (le clock stretching est peu fiable sur ESP32 classique), lancée une fois au démarrage, puis lecture de 9 octets par cycle avec vérification CRC, application de l'offset de zéro. Compteurs d'erreurs I2C/CRC.
3. Zéro : bouton GPIO27 (pull-up interne, actif à la masse) qui lance une moyenne de N échantillons à l'arrêt ; l'offset est conservé en mémoire non volatile ; confirmation par la LED de la carte (GPIO2).
4. BLE central : Cycling Power (0x1818 / 0x2A63) et CSC (0x1816 / 0x2A5B) ; filtrage par nom ou adresse MAC (ne pas se connecter au premier venu : home trainer, etc.) ; reconnexion automatique.
5. Log binaire compact sur LittleFS (pas SPIFFS ni FFat), fichier par session, partition de données personnalisée (environ 2,5 à 2,9 Mo au plus avec 4 Mo de flash). **Format à trancher** : 16 o × 20 Hz × 5 h = 5,8 Mo ne tient pas. Options : 10 Hz avec enregistrements de 10 à 12 octets (deltas sur 16 bits), microSD (vibrations), ou ESP32-S3 16 Mo.
6. Récupération des données : dump série par script, à travers le micro-USB (pont USB-série de la carte) ; pas d'USB natif, donc pas de mode USB Mass Storage ; BLE réservé à du statut léger (débit insuffisant pour des Mo).
7. Écran OLED SSD1306 (0x3C) sur le bus B : mise à jour par pages de 128 octets, une page par tour de boucle, pour ne pas retarder la lecture du capteur n°2 ; quelques valeurs (V_air, lacet, puissance, état BLE/zéro). Sharp Memory LCD (SPI) en option pour la version finale. Boutons GPIO14 et GPIO13 (pull-up interne, actifs à la masse) : fonctions à définir.
8. Mise à jour : flashage par le micro-USB (pont USB-série, reset automatique) ; GPIO0 reste le bouton BOOT de la carte.
9. Vitesse roue à partir de la circonférence mesurée par rollout. Fréquence d'échantillonnage liée au choix du format de log (point 5).
10. Mesure batterie : ADC1 (GPIO35), pont 2 × 100 kΩ, moyenne glissante, lecture à calibrer contre un multimètre ; arrêt propre du log vers 3,3 V.
11. Consommation : mesurer en réel ; régler l'intervalle de connexion BLE.

## Si un capteur est ANT+ uniquement

Option 1 (recommandée) : ne pas l'interfacer ; enregistrer puissance/vitesse sur le compteur habituel (fichier FIT) et recouper avec le log de l'ESP32 par horodatage en post-traitement. Option 2 : carte nRF52840 + SoftDevice BLE+ANT (inscription thisisant.com), nettement plus lourd.

## Post-traitement (à écrire dans `analysis/`)

- Lecture du log binaire et du FIT éventuel.
- Régression de vitesse : √q = k·(V_sol + s·w) par moyenne sur des fenêtres stables ≥ 10 s, sens aller/retour s = ±1, sorties k et w.
- Courbes de lacet : (PG−PD)/(PC−Patm) en fonction de β et Cq(β) à partir des passes sur le support pivotant.
- Densité de l'air à partir de P, T, humidité.
- Bilan de puissance et SCx.
