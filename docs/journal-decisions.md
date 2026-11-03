# Journal des décisions et corrections

Ordre approximativement chronologique. « Erreur » = point où une proposition initiale a été corrigée ; à ne pas reproduire.

## Architecture

- 3 pressions via 2 capteurs différentiels (PC−Patm, PG−PD) plutôt qu'un 3ᵉ capteur. Gamme ±500 Pa choisie contre ±125 Pa (saturation à ~51 km/h).
- Le ventilateur n'est pas retenu comme étalonnage principal (jet non uniforme, anémomètre peu précis à basse vitesse) : calibration en mouvement réel + support pivotant.
- Remarque sur les coefficients : les deux capteurs donnent A = PC−Patm et B = PG−PD. Le rapport B/A est une fonction de β seul, ce qui permet d'inverser β puis q = A/Cq(β). Le K_β classique (PG−PD)/(PC−(PG+PD)/2) n'est pas disponible tel quel avec ces deux mesures : à traiter dans l'analyse.

## Mécanique de la tête

- Orifices : Ø1,3 mm percés après impression (le FDM sous-dimensionne et ovalise). Méplat à 45° dont la normale donne l'axe de perçage.
- Canal coudé en deux droites : remplacé par une droite inclinée par canal + chambre de jonction imprimée ; pas de perçage depuis l'intérieur.
- Erreur : un perçage étagé (Ø1,3 puis Ø3,5 depuis la chambre) a été proposé ; impossible à percer (on ne passe pas un foret de 3,5 par un trou de 1,3). Correction : tout imprimé ouvert, seule une paroi de ~3 mm est percée.
- Erreur : tête Ø18–20 annoncée ; 3 canaux + chambre ne rentrent pas. Retenu : Ø24.
- Raccords barbelés non imprimables en FDM : tenons lisses + serrage du tuyau (gaine thermo ou colle en secours) ; raccords laiton collés comme plan B.
- Tube 12×10 mm retenu (alu ou carbone) pour la structure ; tuyaux silicone à l'intérieur.
- (Version 1, abandonnée) Erreur : tenons plus courts que l'emboîtement. Pour imprimer sans support (nez vers le haut, face arrière sur le plateau), les tenons doivent faire 16 mm = profondeur d'emboîtement.
- Erreur : silicone 2×4 sur tenon Ø3,2 s'étire à ~Ø4,7, trop gros pour trois tuyaux dans un alésage de 10 mm. Retenu : silicone 1,5×3 sur tenon Ø3,0, alésage Ø1,4 (~Ø4,0 étiré, jeu 0,5 mm). Les schémas antérieurs avec Ø3,2/Ø1,6/2×4 sont périmés (la vue arrière de `hardware/tete_pitot.svg` a été corrigée).
- Pneumatique : un coude ne fausse pas la valeur en régime établi, seulement le temps de réponse ; le temps de réponse reste négligeable même avec des tuyaux de Ø1,5.

- Version 2 (3 nov. 2026) : premier essai d'impression des tenons imprimés Ø3,0 : cassure entre les couches au retrait des supports (pilier de 16 mm imprimé debout, couches perpendiculaires à la flexion, paroi 0,8 mm, alésage Ø1,4 impossible à finir). Décision : embouts en tube laiton collés dans des logements imprimés, pas de supports dans l'emboîtement.
- Le plan « SDP81x – Tube Connection » de la datasheet montre des ports Ø3,5 (embout) / Ø5,2 (bourrelet) : la même connectique des deux côtés (tube laiton Ø3,5 sur la tête, tuyau 3×5) supprime les adaptateurs. Trois tuyaux étirés à Ø5,3 ne tiennent pas dans un tube 12×10 ni 14×12 (jeu ≈ 0,1 mm), d'où le tube 16×14 (jeu 0,9 mm). Tête toujours Ø24 (paroi d'emboîtement 3,8 mm). Les cotes 1,5×3 / Ø3,0 / Ø1,4 / tube 12×10 de la version 1 sont abandonnées.
- Temps de réponse pneumatique : négligeable avec ID 3 (< 0,1 ms) comme avec ID 1,5 (≈ 0,2 ms).

## Longueur du tube et pneumatique

- Le tube 16×14 passe de 300 à 400 mm visés (300–500 mm à tester) : la longueur sert à éloigner la tête du blocage amont du vélo, pas à la mécanique. Flexion négligeable (≈ 0,05 mm en bout à 400 mm) ; fréquence propre estimée ≈ 90 Hz à 300 mm, 60 Hz à 400 mm, 40 Hz à 500 mm (tête supposée 20 g, à vérifier). Test : comparer le coefficient k d'étalonnage et B/A en coupant du plus long au plus court.
- Capteurs au niveau de la sonde : sans intérêt (retard pneumatique < 0,1 ms, SDP810 de 29×18×10 mm, fils et exposition).
- Orifice Ø1,3 : compromis dans la plage 1,2–1,5 (ni colmatage ni trop de moyennage), pas un résultat de calcul.
- Modèle théorique sphère potentielle ajouté dans `docs/01` et `analysis/theory_curves.py`.

## Électronique

- Erreur : l'utilisateur pensait brancher les capteurs sur les entrées analogiques ; les SDP810/811 sont purement I2C. Les capteurs analogiques (MPXV7002DP + ADC ESP32) auraient mal résolu 75 Pa.
- 3,3 V pour les capteurs (pas 5 V : SDA/SCL > 3,6 V hors tolérance des GPIO).
- Une seule paire de pull-ups par bus.
- Carte SD abandonnée (vibrations) : LittleFS interne ; donc ESP32-S3 16 Mo (4 Mo insuffisent pour 5 h, même en binaire). (Remplacé le 3 nov. 2026 : DevKit V1 4 Mo pour le prototype, format de log à trancher.)
- Écran : Sharp Memory LCD plutôt qu'OLED (soleil, consommation) pour la version finale ; l'OLED I2C du kit de dev reste utilisable pour les tests (pas de conflit d'adresse, bus séparé conseillé).
- Câblage en fils souples (décision de l'utilisateur, rappelée le 3 nov. 2026) : ni PCB, ni plaque à bandes, ni breadboard. La mention initiale de plaque à bandes dans les docs était périmée.
- Générer un PCB fabricable n'est pas possible sans outil EDA/DRC : ne pas le promettre.
- Squelette de code : mesure déclenchée avec clock stretching, peu fiable sur ESP32 classique → mesure continue à privilégier.

## Étalonnage

- Vitesse : allers-retours + régression √q = k·(V_sol + s·w) (ne pas moyenner les deux sens).
- Lacet : support pivotant indexé (13 crans, 5°, R = 60 mm), sur galerie de voiture, puis validation sur vélo.
- Le support pivotant est un dessin ; rien n'est construit.

## Ouvert au moment de l'export (1er nov. 2026)

- SDP811 trop cher chez l'utilisateur : proposition 2 × SDP810 sur deux bus I2C (non validée).
- Modèle exact du kit ESP32, type des capteurs de puissance/vitesse, circonférence de roue.
- Coordonnées CAO : table fournie dans `docs/02-tete-sonde.md`, à vérifier au moment de la modélisation.

## 3 nov. 2026 : capteurs et carte

- 2 × SDP810-500Pa validés et commandés (SDP811 trop cher) : même adresse 0x25, donc un capteur par bus I2C, bus A GPIO21/22 et bus B GPIO25/26.
- Kit de l'utilisateur : ESP32 DevKit V1 (classique, flash 4 Mo). Conséquences : mesure continue du SDP810 (clock stretching peu fiable), dump des logs par le pont USB-série, et 20 Hz × 16 o sur 5 h ne tient pas dans la flash (format de log à trancher).
- OLED du kit de dev sur le bus B (0x3C), avec le SDP810 n°2.

## 3 nov. 2026 : connecteurs et encombrement

- Pas de connecteur Dupont à 90° trouvé : décision de rester sur fils souples (silicone 26–28 AWG) et Dupont droits, fil replié derrière le boîtier du connecteur. Version définitive : fils soudés sur les picots de l'ESP32 si la hauteur le demande.
- Encombrement revu : fond de boîtier ≈ 85×60 mm, hauteur ≈ 30 mm en prototype, ≈ 28 mm en version soudée ; l'ancienne cible 80×50×25 mm est trop petite (voir `docs/04-boitier.md`).
- Batterie (LiPo plate 2000 mAh ou 18650 3000 mAh) : toujours ouverte.

## 3 nov. 2026 : implantation du prototype

- Plan détaillé du boîtier avec DevKit V1 (`hardware/implantation_devkit.svg`) : ≈ 103×73×36 mm en Dupont droits (fil silicone replié), ≈ 25 mm de haut en fils soudés. Ces valeurs remplacent les « 90×70×30 » et « 85×60×28 » annoncés plus tôt, qui supposaient un remplissage plus dense.
- Piste évoquée pour la version finale : XIAO ESP32S3 (21×17,5 mm, 8 Mo de flash, deux bus I2C possibles), boîtier estimé ≈ 70×55×23 mm ; ESP32-C3 écartés (un seul contrôleur I2C d'après nos informations). Non décidé.
