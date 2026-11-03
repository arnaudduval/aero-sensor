# aero-sensor

Capteur aérodynamique DIY pour vélo : une sonde Pitot 3 trous et un enregistreur, pour mesurer le SCx (CdA) d'un cycliste en conditions réelles.

## Principe

Une tête de sonde imprimée en 3D (Ø24 mm, un orifice axial et deux à ±45°) est montée à l'avant du vélo, au bout d'un tube 16×14 mm d'environ 400 mm. Trois tuyaux silicone amènent les pressions à deux capteurs de pression différentielle Sensirion SDP810-500Pa :

- orifice central − pression statique → vitesse de l'air ;
- orifice gauche − orifice droit → angle de lacet (vent de travers).

Un ESP32 lit les capteurs, récupère en BLE la puissance et la vitesse roue des capteurs du vélo, et enregistre tout en flash. Le SCx se calcule ensuite par bilan de puissance.

## État

Projet en cours de conception, prototype en préparation.

- Tête de sonde : version 2 (embouts laiton collés) dessinée et modélisée sous FreeCAD ; la version 1 imprimée a montré que les tenons imprimés cassaient.
- Électronique : ESP32 DevKit V1, 2 × SDP810 commandés, câblage et alimentation (LiPo, TP4056, boost) définis ; rien monté.
- Firmware : un premier squelette (`firmware/legacy_cda_logger/`) à réécrire, et un sketch de prise en main BLE + OLED (`firmware/sandbox/hrm_logger/`).
- Analyse : courbes théoriques de la sonde ; scripts d'étalonnage et de calcul du SCx à écrire.

## Contenu

| Dossier | Contenu |
|---|---|
| `docs/` | Physique et étalonnage, tête de sonde, électronique, boîtier, firmware, journal des décisions |
| `hardware/` | Schémas SVG (tête, câblage, implantation, support pivotant d'étalonnage) et modèle 3D (`hardware/3D/`) |
| `firmware/` | Code ESP32 |
| `analysis/` | Post-traitement et courbes théoriques |

## Licence

Voir [LICENSE](LICENSE).
