# Schémas

| Fichier | Contenu | État |
|---|---|---|
| `tete_pitot_v2.svg` | Version 2 de la tête : coupe A–A par l'axe des deux embouts latéraux (tube 16×14, embouts laiton Ø3,5 collés, tuyaux 3×5), zoom ×3 sur un embout, vue arrière avec la trace du plan | **à jour** |
| `support_pivotant.svg` | Support pivotant à crans pour l'étalonnage du lacet : vue de dessus (13 crans, 5°, ±30°, R = 60 mm, pion Ø3, pivot M5) et coupe selon l'axe du bras | à jour |
| `cablage_complet.svg` | Schéma de câblage complet avec les broches : LiPo, TP4056, interrupteur, boost MT3608, D1, C3, ESP32 DevKit V1, 2 × SDP810, OLED, boutons, pont de mesure de batterie | **référence de câblage** |
| `cablage_noeuds.svg` | Nœuds par broche de l'ESP32 : un fil par broche, branches, R1 à R6, C3, C6, D1 | à jour (complète `cablage_complet.svg`) |
| `implantation_devkit.svg` | Implantation du boîtier prototype avec DevKit V1 : vue de dessus et coupe, cotes (≈ 103×73×36 mm, ≈ 25 mm en fils soudés) | à jour |
| `cablage_alim.svg` | Chaîne d'alimentation : USB-C → TP4056 protégé ↔ LiPo → interrupteur → boost 5,1 V → D1 → VIN | à jour |
| `cablage_signaux.svg` | Signaux : ESP32 DevKit V1, 2 × SDP810 sur 2 bus I2C, OLED sur le bus B, boutons, mesure batterie | à jour |
| `3D/Sonde.FCStd` | Modèle FreeCAD de la tête v2 (3 nov. 2026) ; le STL s'exporte depuis ce fichier et n'est pas versionné | à confronter aux cotes de `docs/02-tete-sonde.md` |
| `archive_v1/tete_pitot_v1.svg`, `archive_v1/tete_pitot_tenons_v1.svg` | Version 1 (tenons imprimés Ø3,0, tube 12×10, tuyau 1,5×3) | **périmé**, conservé pour l'historique |

Les SVG utilisent les variables et classes du widget de la conversation d'origine (`var(--t)`, classes `th`, `ts`, `leader`, `arr`, `c-gray`, marqueur `#arrow`). Pour les visualiser hors de ce contexte, ajouter un bloc `<style>` définissant ces classes (texte 12 et 14 px, traits fins, `rect.c-gray` en gris) et remplacer `var(--t)` par une couleur, par exemple `#0F6E56` ; `cairosvg` (pip) convient pour un rendu PNG.

Code couleur : vert = passage d'air imprimé ouvert, corail = percé après impression, violet = tuyau silicone, ambre = laiton et époxy, gris = tube.
