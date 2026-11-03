# Conception de la tête de sonde (version 2)

Version 2 (3 nov. 2026) : le premier essai d'impression a montré que les tenons imprimés Ø3,0 mm (piliers de 16 mm imprimés debout) cassent entre les couches, notamment au retrait des supports. Ils sont remplacés par des embouts en tube laiton collés dans des logements imprimés, et le tube support passe de 12×10 à 16×14 pour utiliser le même tuyau silicone 3×5 de bout en bout, sans adaptateur côté boîtier. L'historique de la version 1 est dans `journal-decisions.md` et `hardware/archive_v1/`.

Toutes les cotes en mm. Repère (convention proposée pour la CAO) : z = distance depuis le bout du nez vers l'arrière (axe de la tête), y = latéral, x = vertical (+ haut ; l'embout central est en haut). Source graphique : `hardware/tete_pitot_v2.svg` (coupe A–A, zoom ×3, vue arrière), à l'échelle 6 px/mm (nez en x = 600, axe en y = 170).

## Géométrie extérieure

- Nez hémisphérique R = 12 (Ø24), centre à z = 12.
- Cylindre Ø24 de z = 12 jusqu'à la face arrière plane z = 74 (le cône de la version 1 n'est plus nécessaire).
- Emboîtement pour le tube 16×14 : Ø16,4 × 16 de profondeur (z = 58 à 74), fond plat à z = 58, paroi de 3,8 mm.
- Tube 16×14 (alu ou carbone) collé à l'époxy (jeu radial 0,2 mm) + goupille transversale Ø2 ; méplat de calage à plat sur le dessus.

## Orifices (percés après impression, Ø1,3)

| Orifice | Position sur le nez | Axe de perçage | Paroi à percer |
|---|---|---|---|
| Axial | z = 0, sur l'axe | parallèle à z | ≈ 3,6 mm |
| Gauche (+y) | sortie en z ≈ 3,5, y ≈ +8,5 | 45° de l'axe, passe par le centre de la chambre G | ≈ 3,6 mm |
| Droit (−y) | symétrique | symétrique | idem |

- Méplat de ~3–4 mm de diamètre à 45° autour de chaque sortie latérale, normale du méplat = axe de perçage (pas de perçage de biais).
- Symétrie gauche/droite plus importante que l'angle exact : fonction miroir en CAO.
- Pré-percer à Ø0,5–0,8 puis Ø1,3, ébavurer, arête vive, ne pas chanfreiner. PLA ramollit vers 60 °C : percer lentement.

## Chambres et canaux (imprimés ouverts, sans perçage)

| Élément | Diamètre | Position |
|---|---|---|
| Chambre G / D | Ø4 | z = 7,5 ; y = ±4,5 ; x = 0 |
| Canal latéral G | Ø2,8 | de la chambre (z 7,5 ; y +4,5 ; x 0) à z = 50, sur l'axe du logement G (y +3,0 ; x −1,73), puis parallèle à z dans le logement |
| Canal latéral D | Ø2,8 | symétrique |
| Canal central | Ø2,8 | de z ≈ 3,6 (x = 0) à z = 50 sur l'axe du logement central (y 0 ; x +3,46) ; il quitte le plan A–A vers z ≈ 18 |

- Les canaux sont des droites inclinées (≈ 3° pour les latéraux, ≈ 4° pour le central), sans coude interne ; section en goutte d'eau ou losange si le porte-à-faux approche 45°.
- Les 8 derniers mm de chaque canal sont parallèles à l'axe de la tête pour être coaxiaux avec le logement de l'embout.

## Embouts laiton (pièces rapportées, collées à l'époxy)

- Tube laiton Ø3,5 extérieur, intérieur Ø2,5 à 3,05 (K&S 3,5 × 300 : paroi annoncée 0,225 mm, donc paroi fine à couper à la scie fine et à ébavurer avec soin ; l'épaulement Ø3,7 → Ø2,8 du logement sert de butée), longueur 18 mm : 10 mm dépassent du fond de l'emboîtement, 8 mm sont collés dans le logement. Tube coupé net, ébavuré.
- Logement : trou Ø3,7 imprimé axe vertical (nez vers le haut, trous les plus ronds dans cette orientation), profondeur 8 mm depuis le fond (z = 58 vers z = 50), débouche dans le canal Ø2,8. Ajuster le diamètre d'après la pièce test ; au besoin aléser à la main depuis l'emboîtement.
- Positions (triangle équilatéral de côté 6,0, centré sur l'axe) : central (x +3,46 ; y 0) ; latéraux (x −1,73 ; y ±3,0). Matière entre deux logements : 6,0 − 3,7 = 2,3.
- Époxy sur toute la surface du logement, joint externe visible, puis test d'étanchéité.
- Tuyau silicone 3×5 sur l'embout Ø3,5 : étiré à environ Ø5,3 (√(3,5² + (5² − 3²)) = 5,31), recouvrement 10 mm. Même embout Ø3,5 que les ports des SDP81x côté boîtier. Option anti-glissement : petit bourrelet de soudure à l'étain près de l'extrémité du tube laiton, ou bague thermorétractable.
- Jeu des tuyaux dans l'alésage Ø14 du tube : 0,9 mm au plus serré (les trois tuyaux étirés tiennent dans Ø11,4 minimum).
- À chercher : tube laiton Ø3,5 mm (ou 9/64" = 3,57 mm) ; un Ø3,0 serait trop lâche dans un tuyau de 3 mm de diamètre intérieur. Le tube laiton Ø3 × 1000 de Scientific France (4,90 €) a été repéré mais n'a pas la bonne cote.

## Impression

- Orientation : nez vers le haut, face arrière (z = 74) sur le plateau. **Supports désactivés** dans l'emboîtement : le fond (z = 58) est un pont de 16,4 mm. Refroidissement à fond, pont lent. Si le pont s'affaisse un peu, ce n'est pas critique : le tube bute sur le pourtour et les embouts sont collés dans des logements verticaux.
- PLA standard suffisant maintenant que les pièces fines sont en laiton (le PETG est possible).
- Nez imprimé axe vertical : artefacts de couches axisymétriques donc symétriques gauche/droite ; léger ponçage du nez.
- Ne jamais compter sur un trou horizontal < 2 mm imprimé : percer.

## Pièce test (à imprimer avant la tête)

Cylindre Ø24 × 24 mm, face arrière sur le plateau :
- emboîtement Ø16,4 × 16 de profondeur ;
- 3 logements Ø3,7 traversants (8 mm, au-dessus du fond) en triangle de côté 6,0 : central (x +3,46 ; y 0), latéraux (x −1,73 ; y ±3,0) ;
- aucun support.

À vérifier : pont du fond, diamètre réel des logements (bague test avec le laiton Ø3,5), tenue du collage (époxy), passage de l'air dans les embouts, enfilage du tuyau 3×5, jeu du tube 16×14 dans l'emboîtement.

## Ordre d'assemblage

1. Pièce test, puis tête complète ; percer les trois orifices (Ø0,5–0,8 puis Ø1,3), ébavurer.
2. Coller les trois embouts laiton dans leurs logements (époxy), en les alignant avec l'axe ; laisser durcir.
3. Test d'étanchéité canal par canal (boucher le tuyau et vérifier que la lecture ne dérive pas).
4. Passer les 3 tuyaux 3×5 dans le tube 16×14 avec un fil de traction, les enfiler sur les embouts (10 mm).
5. Glisser le tube dans l'emboîtement, époxy, goupille Ø2.
6. Côté boîtier : enfiler les tuyaux directement sur les ports Ø5,2 des SDP.

## Pour la CAO (OpenSCAD/CadQuery)

Paramètres : R_nez = 12 ; L_tete = 74 ; Ø_emboîtement = 16,4 ; prof_emboîtement = 16 ; Ø_orifice = 1,3 ; épaisseur_paroi = 3,6 ; Ø_chambre = 4 ; Ø_canal = 2,8 ; z_chambre = 7,5 ; y_chambre = 4,5 ; Ø_logement = 3,7 ; prof_logement = 8 ; Ø_embout = 3,5 ; L_embout = 18 ; dépassement = 10 ; entraxe = 6,0 ; positions = central (x +3,46 ; y 0), latéraux (x −1,73 ; y ±3,0).

## Règles FDM retenues

- Buse 0,4 : un trou horizontal de Ø < 2 mm sort ovalisé et plus petit de 0,2–0,4 mm.
- Les dents d'un raccord barbelé ne sont pas imprimables ; les piliers fins imprimés debout cassent entre les couches : toute pièce fine et sollicitée en flexion est en métal rapporté.
- À 10–100 Pa l'étanchéité est facile ; le risque est l'obstruction ou le décollement par vibrations.
- Ne jamais imprimer un canal borgne de 300 mm : le tube rigide est structurel, les tuyaux silicone sont remplaçables.
