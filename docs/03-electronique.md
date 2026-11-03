# Électronique

## Capteurs de pression Sensirion SDP810 / SDP811

- Gamme ±500 Pa, sortie I2C numérique 16 bits (pas de sortie analogique : ne pas les brancher sur les entrées ADC de l'ESP32).
- Alimentation 2,7–5,5 V ; utiliser **3,3 V** (SDA/SCL max = VDD + 0,3 V, donc en 5 V le bus dépasserait ce que tolèrent les GPIO de l'ESP32).
- I2C jusqu'à 1 MHz ; 20 Hz passe très largement.
- Adresses : SDP810 = 0x25, SDP811 = 0x26 (même capteur, adresse différente). Deux SDP810 sur le même bus entrent en conflit.
- Ports pneumatiques (plan « SDP81x – Tube Connection » de la datasheet) : embout Ø3,5 ±0,2 mm, bourrelet Ø5,2 ±0,2, collerette Ø4, bride Ø8 ; deux ports « High » et « Low » espacés de 12,6 mm ; longueur de port 13,5 mm (bourrelet à 9,65) ; boîtier 29 ±0,4 × 18 ±0,4 mm, hauteur du corps 10,25 mm ; trous de fixation Ø2,9 à 24 mm d'entraxe. Un tuyau silicone de 3 mm de diamètre intérieur (3×5) s'enfile directement sur ces ports. Pour PC−Patm : High = PC, Low laissé à l'air libre dans le boîtier.
- Connecteur 4 broches pas 2,00 mm ; ordre physique vu de dessous : SDA – GND – VDD – SCL. Brochage : 1 SCL, 2 VDD, 3 GND, 4 SDA.
- Pas de pattes à souder : prendre le **SDP800-SERIES-CAP** (capuchon + câble, ~1,50 € chez Digikey) pour câbler. Le capteur n'est **pas soudable au reflow** (vague seulement) : en cas de PCB assemblé, le préciser à l'assembleur.
- Prix indicatif Digikey FR : ~28 € (SDP810), ~27 € (SDP811 selon la page produit consultée ; l'utilisateur signale depuis un SDP811 beaucoup plus cher).
- Zéro : routine de zéro à l'arrêt avant chaque sortie.

### Décision : 2 × SDP810 sur deux bus I2C (validé et commandé le 3 nov. 2026)

Le SDP811 étant trop cher, l'utilisateur a commandé 2 × SDP810 (0x25), chacun sur son bus.

- Bus A : `Wire.begin(SDA_A, SCL_A)` ; Bus B : `Wire1.begin(SDA_B, SCL_B)` (l'ESP32 a deux contrôleurs I2C ; sur le S3 les broches sont libres).
- Un jeu de pull-ups 4,7 kΩ vers 3,3 V par bus (donc 4 résistances au total au lieu de 2).
- Alternatives : multiplexeur TCA9548A ; SDP801-500Pa (adresse 0x26, non vérifié) ; famille SDP3x (SDP31 : 0x21 par défaut, 0x22 avec 1,2 kΩ sur ADDR, 0x23 avec 2,7 kΩ — QFN petit, peu de cartes disponibles, donc moins pratique).

### Mode de mesure

Le squelette utilise la mesure déclenchée avec clock stretching. Sur ESP32 classique le clock stretching est peu fiable : préférer la mesure continue en pression différentielle (lancer une fois, puis lire 9 octets par cycle). Codes de commande à vérifier dans la datasheet (de mémoire : continue pression différentielle moyennée 0x3615, non moyennée 0x361E, arrêt 0x3FF9 ; déclenchée avec stretching 0x372D).

## Câblage logique (ESP32 DevKit V1)

Schéma : `hardware/cablage_complet.svg` (et `cablage_signaux.svg` pour les seuls signaux). Ordre des lignes sur les bus : SDA, 3V3, SCL, GND, avec deux pull-ups de 4,7 kΩ en chaîne SDA–3V3–SCL (R1, R2 bus A ; R3, R4 bus B). Broches choisies hors broches de démarrage (0, 2, 5, 12, 15), hors flash (6 à 11) et hors entrées seules (34 à 39, sauf pour la mesure ADC).

| Signal | Broche ESP32 | Liaison |
|---|---|---|
| Bus A SDA / SCL | GPIO21 / GPIO22 (Wire) | SDP810 n°1 (0x25) ; 2 × 4,7 kΩ vers 3V3 |
| Bus B SDA / SCL | GPIO25 / GPIO26 (Wire1) | SDP810 n°2 (0x25) et OLED SSD1306 (0x3C) ; 2 × 4,7 kΩ vers 3V3 |
| 3V3, GND | broches 3V3 et GND de la carte | VDD et GND des SDP810 et de l'OLED, 100 nF au plus près de chaque VDD |
| Bouton zéro | GPIO27 | vers GND, pull-up interne |
| Boutons 2, 3 | GPIO14, GPIO13 | vers GND, pull-up interne |
| Mesure batterie | GPIO35 | pont 2 × 100 kΩ + 100 nF |
| LED | GPIO2 | LED de la carte |
| BOOT | GPIO0 | bouton BOOT de la carte, sert pour le flashage |

- L'OLED est branché en parallèle sur les fils du bus B (même SDA, SCL, 3V3 et GND que le SDP810 n°2) : dérivation soudée ou connecteur à levier sur chacun des quatre fils. Alimentation VCC de l'OLED : 3V3, pas le 5 V.
- Les modules OLED ont souvent leurs propres pull-ups (4,7 à 10 kΩ) ; en parallèle avec 4,7 kΩ, cela reste correct. Le SDP800-SERIES-CAP n'a pas de pull-ups.
- Le bus B porte le SDP810 n°2 et l'OLED : un rafraîchissement complet de l'OLED (environ 1 Ko, 20 à 25 ms à 400 kHz) retarde la lecture du capteur n°2. Mettre à jour l'écran par pages de 128 octets (une page par tour de boucle), ou déplacer l'OLED sur le bus A.
- Brochage du connecteur du SDP810 : 1 SCL, 2 VDD, 3 GND, 4 SDA (ordre physique vu de dessous : SDA, GND, VDD, SCL). À contrôler sur la datasheet et au multimètre avant de mettre sous tension, la couleur des fils du capuchon n'étant pas documentée ici.
- Câblage souple sans PCB : fil silicone 26 à 28 AWG, connecteurs Dupont ou JST-PH 2,0 selon les modules, reprise de contrainte (colle chaude) sur chaque fil, soudures sur picots plutôt que fils volants en l'air.

## Alimentation (version retenue le 3 nov. 2026 : LiPo plate + TP4056 + boost)

Schéma complet avec les broches : `hardware/cablage_complet.svg` (référence). Schéma détaillé de l'alimentation : `hardware/cablage_alim.svg`. Le micro-USB du DevKit V1 n'a ni chargeur ni entrée batterie (il alimente VIN et l'AMS1117) : la charge se fait par le port USB-C du module TP4056, le micro-USB de l'ESP32 reste pour le flashage et le dump des logs.

Chaîne : port USB-C du TP4056 → TP4056 → LiPo ; sortie protégée OUT+/OUT− → interrupteur sur OUT+ → boost 5,1 V → diode Schottky D1 → broche VIN de l'ESP32 → AMS1117 de la carte → 3V3 (capteurs et OLED).

- Batterie : LiPo plate 1S 3,7 V, 2000 mAh, avec sa propre protection (PCM) si possible ; encombrement typique d'environ 6×50×60 mm (réf. 605060) ou 10×40×50 mm (104050), à vérifier à l'achat. Prise de câblage de polarité souvent inversée sur les LiPo bon marché : vérifier + et − au multimètre avant de brancher.
- Chargeur : module TP4056 **avec protection** (6 pastilles B+, B−, OUT+, OUT−, puces DW01A + 8205A), entrée USB-C. Le courant de charge par défaut (résistance de 1,2 kΩ, environ 1 A) vaut 0,5 C pour 2000 mAh : alimentation USB d'au moins 1 A. Charger avec l'interrupteur ouvert : le TP4056 n'est pas un chargeur à partage de charge, et avec l'ESP32 allumé la charge ne se termine pas (la LED ne passe pas au vert).
- Boost : module de type MT3608. **Régler le trimmer à 5,1 V à vide, avant de connecter l'ESP32** (réglage d'usine souvent très supérieur à 5 V). 5,1 V − 0,3 V (D1) laisse environ 4,8 V à VIN, assez pour l'AMS1117 (chute de tension environ 1,1 V).
- D1 : diode Schottky (1N5817 ou SS14) entre la sortie du boost et VIN, pour que le 5 V de la batterie ne remonte pas vers le port USB du PC. Vérifier au multimètre si la carte a déjà une diode entre VBUS et VIN.
- Condensateur de 220 µF sur VIN (pics de courant du BLE) ; 100 nF et 10 µF près des deux SDP810.
- Mesure de la batterie : pont de deux résistances de 100 kΩ pris après l'interrupteur (donc sans courant quand éteint), milieu vers GPIO35 (ADC1) avec 100 nF vers la masse. Le firmware arrête le log proprement vers 3,3 V ; la lecture ADC du ESP32 n'est pas linéaire, à calibrer par comparaison avec un multimètre.
- Consommation : estimation de 150 mA à 3,3 V (BLE central avec deux périphériques, deux SDP810 d'environ 4 mA chacun, OLED 10–20 mA), à mesurer. Avec le rendement du boost (environ 90 %) et l'AMS1117 (linéaire, 5 V → 3,3 V), le rendement global est d'environ 60 % : environ 1,1 Ah pour 5 h, d'où 2000 mAh avec marge. Alternative d'efficacité (version ultérieure) : buck-boost 3,3 V sur la broche 3V3, environ 750 mAh pour 5 h, mais l'AMS1117 peut alors recevoir un retour de courant (VIN à 0 V, sortie à 3,3 V) : non retenu pour l'instant.
- Charge pendant que l'ESP32 est actif : à éviter (voir ci-dessus).

## Écran et boutons

- Pour les premiers tests : l'OLED I2C (SSD1306/SH1106, adresse 0x3C ou 0x3D) déjà présent sur le kit de dev de l'utilisateur, branché sur le bus B avec le SDP810 n°2 (0x3C, pas de conflit d'adresse) ; un rafraîchissement complet (≈ 1 Ko, 20–25 ms à 400 kHz) retarderait la lecture du capteur, d'où la mise à jour par pages (voir plus haut) ; illisible en plein soleil et ≈ 10–20 mA.
- Version finale (optionnelle) : voir ci-dessous.

- Sharp Memory LCD 1,3" 96×96 (Adafruit ; option 168×144) : réflectif, lisible en plein soleil, rafraîchissement rapide, SPI 3 fils, logique 3 V. Pas d'OLED (illisible au soleil).
- 3–4 boutons au total (zéro + 2–3 libres), fonctions à définir.

## Montage : câbles souples, sans PCB ni plaque à bandes

- Tout est relié par des fils souples (silicone 26 à 28 AWG) entre les modules, sans circuit imprimé ni plaque à bandes. Les modules gardent leurs broches d'origine.
- Liaisons enfichables (Dupont femelles) : OLED, boutons, DevKit V1 pour les signaux. Reprise de contrainte sur chaque connecteur (gaine thermorétractable ou colle chaude) à cause des vibrations. Le connecteur du SDP800-SERIES-CAP est à pas 2,00 mm : vérifier à la réception s'il s'enfiche sur des Dupont (pas 2,54 mm) ou s'il faut un boîtier JST-PH 2,0 ; ne pas forcer.
- Liaisons soudées : batterie, TP4056, interrupteur, boost, D1, pont de mesure de batterie, et tous les points de jonction.
- Nœuds à plusieurs branches (3V3, GND, et SDA/SCL du bus B) : soudures en dérivation avec gaine thermorétractable, ou un connecteur à levier de type WAGO 221 pour les prototypes. Il y a un seul 3V3 sur le DevKit V1, d'où la dérivation vers SDP810 n°1, n°2, OLED et pull-ups. Plusieurs broches GND sont disponibles sur la carte.
- Composants passifs soudés directement entre les fils, sans support : 4 résistances de 4,7 kΩ (pull-ups au plus près de l'ESP32), 2 résistances de 100 kΩ (pont de batterie), condensateurs de 100 nF et 10 µF au niveau de chaque VDD des SDP810, 220 µF entre VIN et GND côté ESP32. Chaque composant est isolé par une gaine thermorétractable.
- Pas de breadboard (contacts à friction, vibrations).
- Si le câblage devient trop encombrant dans le boîtier, un petit morceau de plaque perforée de 20×30 mm en guise de répartiteur 3V3/GND reste possible : option, pas retenue pour l'instant.
- PCB dédié seulement après validation du prototype (KiCad ; les empreintes SDP810/811 sont fournies par Digikey « Modèles EDA/CAO »).

## Connecteurs et nœuds (broches de l'ESP32, OLED, SDP)

Schéma : `hardware/cablage_noeuds.svg`. Règle : un seul fil par broche de l'ESP32, qui va vers un nœud (épissure soudée et gainée, ou WAGO 221 en doublant le brin de fil fin), puis les branches.

- Côté modules à broches (ESP32, OLED, éventuellement SDP sur carte) : Dupont femelles d'une seule broche sur l'ESP32 (les signaux sont répartis sur les deux rangées), boîtier de 4 pour l'OLED (ordre de la sérigraphie à vérifier avant de brancher). SDP810 nu : connecteur propre (pas 2 mm probable) et câble SDP800-SERIES-CAP, brochage à vérifier sur la fiche technique.
- Nœuds : 3V3 (câble A, câble B puis Y vers l'OLED) ; GND signaux (câbles A et B, boutons) ; GND puissance (TP4056 OUT−, boost, C3, R6 et C6) sur la broche GND voisine de VIN ; GPIO25 et GPIO26 (SDP n°2, OLED, pull-up R3 ou R4) ; GPIO21 et GPIO22 (SDP n°1, pull-up R1 ou R2) ; VIN (D1, C3) ; GPIO35 (R5, R6, C6).
- Pull-ups : chaque résistance relie le fil de signal au fil 3V3 du même câble, côté ESP32. Découplage : C1/C4 et C2/C5 entre VDD et GND, côté capteur, à moins de 3 cm du connecteur.
- Fils : 28 AWG pour les signaux, 24 à 26 AWG pour la chaîne de puissance. Gaine thermo enfilée avant la soudure ; point de colle pour décharger les pattes ; décaler les épissures.
- Dupont sur un vélo : gaine thermo sur le boîtier du connecteur, point de colle contre la carte, câble attaché au boîtier.
- Avant mise sous tension : continuité, pas de court-circuit 3V3/GND, boost réglé à 5,1 V, sens de D1 et polarité de C3.

## BOM (état actuel)

| Réf. | Composant | Qté | Remarque |
|---|---|---|---|
| U1, U2 | SDP810-500Pa | 2 | commandés par l'utilisateur ; un par bus I2C |
| — | SDP800-SERIES-CAP (capuchon + câble) | 2 | une par capteur |
| R1, R2, R3, R4 | 4,7 kΩ (noté 4k7) | 4 | pull-ups : R1, R2 sur le bus A, R3, R4 sur le bus B |
| C1, C2 | 100 nF céramique, de préférence X7R, à pattes, ≥ 16 V (50 V courant) | 2 | découplage VDD, un par SDP810 ; non polarisé |
| U3 | ESP32 DevKit V1 (ESP32 classique, flash 4 Mo) | 1 | kit de l'utilisateur ; un ESP32-S3 16 Mo reste une option si le log ne tient pas |
| BT1 | LiPo plate 1S 2000 mAh (avec PCM) | 1 | à valider par mesure de consommation |
| U4 | Module TP4056 USB-C avec protection (DW01A + 8205A) | 1 | charge à 1 A par défaut |
| DISP1 | OLED SSD1306 (kit de dev de l'utilisateur) | 1 | Sharp Memory LCD en option pour le plein soleil |
| SW1–SW3 | Boutons poussoirs | 3 | SW1 = zéro (GPIO27), SW2 (GPIO14), SW3 (GPIO13) ; retour commun au nœud GND signaux |
| LED1 | LED de la carte (GPIO2) | — | rien à acheter |
| — | Fil silicone 26–28 AWG, Dupont femelles, gaine thermorétractable, connecteurs à levier (WAGO 221) | — | pas de PCB ni de plaque à bandes |
| — | Tuyau silicone 3×5 mm | ~2 m | 3 longueurs d'environ 550 mm pour le tube de 500 mm, recoupées avec le tube, directement sur les ports Ø5,2 des SDP, aucun adaptateur |
| — | Tube laiton Ø3,5 (K&S 3,5 × 300 mm, paroi annoncée 0,225 mm donc intérieur ≈ 3,05 ; ou 9/64") | ~60 mm | 3 embouts de 18 mm ; l'utilisateur source sur AliExpress |
| — | Tube alu 16×1 (intérieur 14) ou carbone 16/14, 1 m | 1 | coupé du plus long (500 mm) au plus court (300 mm) pour le test de blocage amont |
| — | Époxy bi-composant | — | collage des embouts et du tube |
| U5 | Module boost MT3608 réglé à 5,1 V | 1 | à régler avant de connecter l'ESP32 |
| D1 | Diode Schottky 1N5817 ou SS14 | 1 | boost → VIN |
| SW0 | Interrupteur à glissière | 1 | sur OUT+ du TP4056 |
| R5, R6 | 100 kΩ | 2 | pont de mesure de batterie (GPIO35) |
| C3 | 220 µF électrolytique radial faible ESR (105 °C) ou polymère solide, ≥ 10 V (16 V courant) | 1 | sur VIN ; **polarisé** : + côté VIN ; évite le tantale |
| C4, C5 | 10 µF, céramique X7R radiale ≥ 16 V si possible, sinon électrolytique ≥ 16 V | 2 | un par SDP810 ; polarisés seulement en électrolytique (+ côté VDD) |
| C6 | 100 nF céramique X7R à pattes | 1 | sur le point milieu du pont de batterie ; doit rester en céramique (pas d'électrolytique : fuite) |

Notes sur les condensateurs : X7R = stable de −55 à +125 °C à ±15 %, à préférer ; éviter Y5V et Z5U. L'ESR (résistance série équivalente) doit être faible pour un condensateur de découplage. Un lot de 100 nF à 50 V à pattes (lot de 100 pièces ou assortiment de 600 pièces) convient pour C1, C2 et C6 même sans X7R annoncé (3,3 V sur 50 V, température < 85 °C) ; les 10 µF et le 220 µF sont à acheter à part.
