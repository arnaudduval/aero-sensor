# Boîtier

Un seul boîtier à l'autre bout du tube 16×14 (400 mm visés, 300–500 à tester) : électronique, batterie, écran, boutons, et fixation du tout au vélo.

## Besoins posés par l'utilisateur

- Pas d'étanchéité (temps sec uniquement, la pluie perturberait le Pitot) : l'air interne sert de référence Patm pour le capteur PC−Patm, aucun évent à prévoir.
- Batterie interne (LiPo plate 2000 mAh retenue) pour 5 h, recharge par un port USB-C extérieur.
- Données en flash interne (non volatile), récupérables par USB (ou BLE pour du statut).
- Bouton de zéro, écran et 2–3 boutons supplémentaires.

## Encombrement estimé

| Composant | Estimation |
|---|---|
| SDP810 × 2 | 29×18 mm chacun, hauteur 10,25 (ports 13,5) |
| LiPo plate 2000 mAh | ~50×60×6 mm (605060) ou ~40×50×10 mm (104050), à vérifier à l'achat |
| ESP32 DevKit V1 | ~51×28 mm, hauteur avec picots d'environ 12 mm |
| TP4056 USB-C + boost MT3608 + interrupteur | ~26×17 mm + ~36×17 mm + ~10×5 mm |
| OLED 0,96" du kit (prototype) ; Sharp 1,3" 96×96 en option | ~27×27 mm ; ~40×40 mm |
| Bandeau de boutons | ~40×15 mm |
| Manchon d'entrée du tube | +15–20 mm de longueur |
| Interface quart de tour Garmin/Wahoo | ~43 mm de diamètre sous le boîtier |

Agencement : sandwich, carte au-dessus (écran visible par une découpe dans le capot), batterie dessous.

Implantation détaillée du prototype avec le DevKit V1 : `hardware/implantation_devkit.svg` (vue de dessus et coupe). Cotes typiques, à confirmer sur les pièces reçues.
- Fond de boîtier extérieur ≈ **103×73 mm** (cavité 98×68), manchon non compris. Tube et manchon au milieu du petit côté ; SDP810 n°1 et n°2 empilés côté tube ; DevKit V1 à l'opposé avec l'antenne vers le tube (≥ 45 mm du manchon) et le micro-USB sur la paroi arrière ; boost MT3608 sous l'ESP32 ; TP4056 au coin avec son USB-C sur la paroi arrière ; zone des nœuds (R1–R6, D1), SW0 et C3 dans l'espace restant ; LiPo plate dessous ; OLED et 3 boutons dans le capot.
- Hauteur prototype (Dupont droits, fil silicone replié) ≈ **36 mm** : fond 2,5 + LiPo 7 + plaque 1,5 + Dupont et fils 16 + ESP32 5 + jeu 2 + capot 2,5. Les « 30 mm » annoncés plus tôt étaient une approximation basse.
- Version soudée sur les picots : Dupont 16 → 4 mm, soit ≈ 103×73×25 mm.
- Cellule 18650 (≈ 67×19×19 avec cosses) : environ +12 mm d'épaisseur sous la carte, ou +22 mm en largeur si posée à côté. Choix de batterie encore ouvert.
- À ajouter : manchon d'entrée du tube (+15–20 mm en longueur) et interface Garmin sous le boîtier. Le 80×50×25 mm précédent est trop petit.
- Pistes pour réduire : empiler TP4056 et MT3608 (≈ −15 mm en largeur) ; passer à une carte plus petite type XIAO ESP32S3 pour la version finale (≈ 70×55×23 mm estimé, voir journal).
- À valider par un montage de volumes simples en CAO avant de dessiner le boîtier final.

Seuls le manchon du tube et le port USB-C percent la coque. Les capteurs sont côté tube pour raccourcir les tuyaux ; écran et boutons du côté opposé, face au cycliste.

## Jonction tube–boîtier (structurelle)

- Le tube de 300 à 500 mm crée un bras de levier : la jonction reprend la flexion (traînée + vibrations + chocs). Le tube passe à 16×14 mm (manchon plus gros) ; un manchon rigide indexé (logement imprimé + vis de blocage ou collier) est nécessaire, pas un collage souple.
- L'indexation est importante : toute rotation du tube décale le zéro du lacet. Prévoir un méplat sur le tube ou une clavette imprimée.
- La fixation au vélo : interface quart de tour Garmin/Wahoo sous le boîtier (démontage facile), avec renfort métallique à l'endroit du clip côté potence/guidon plutôt que PLA seul.

## Électronique dans le boîtier

- Antenne BLE de l'ESP32 éloignée du manchon et de toute visserie métallique (alu ou carbone du tube) : quelques cm, côté opposé au tube.
- Pas de carte SD (connecteur peu fiable en vibration) : flash interne LittleFS.
- Ancrages mécaniques indépendants pour batterie et écran.
- Calibration à faire avec le montage complet assemblé.
