# Physique de la sonde et étalonnage

## Principe de la sonde 3 trous

Trois pressions : PC (orifice axial), PG et PD (orifices à ±45°). Deux capteurs différentiels, câblés ainsi :

- Capteur 1 : PC − Patm (Patm = air ambiant dans le boîtier, non étanche, donc référence correcte).
- Capteur 2 : PG − PD.

Deux coefficients adimensionnels, à calibrer empiriquement :

- Coefficient de lacet : K_β = (PG − PD) / (PC − (PG+PD)/2). Quasi indépendant de la vitesse, monotone sur ±30° environ. Au-delà, l'orifice sous le vent décroche et la courbe cesse d'être monotone.
- Coefficient de pression dynamique : Cq(β) = (PC − Patm) / q, avec q = ½ρV². À angle non nul, l'orifice central ne lit plus la pression d'arrêt pure.

Note : le capteur 2 mesure PG−PD, pas PG−(PG+PD)/2. Pour K_β il faut donc PC−(PG+PD)/2, qui n'est pas mesuré directement avec deux capteurs seulement. Le plan retenu s'appuie sur la calibration empirique de deux fonctions de β et de q à partir des deux mesures disponibles (PC−Patm et PG−PD) : on calibre le rapport (PG−PD)/(PC−Patm) en fonction de β, puis Cq(β). À reprendre proprement dans l'analyse (voir `analysis/`).

## Modèle théorique (sphère en écoulement potentiel)

Le nez est assimilé à une sphère dans un fluide parfait. Pour un point dont la normale fait l'angle φ avec la direction d'où vient l'air, Cp = p/q = 1 − (9/4)·sin²φ, soit Cp = (9/4)·cos²ψ − 5/4 en notant ψ l'angle entre la normale de l'orifice et la direction amont. Avec un orifice central (normale sur l'axe) et deux orifices latéraux dont la normale fait ±45° avec l'axe, pour un lacet β :

- central : Cp_C = (9/4)·cos²β − 5/4
- latéral côté vent : Cp_G = (9/4)·cos²(45° − β) − 5/4
- latéral sous le vent : Cp_D = (9/4)·cos²(45° + β) − 5/4

Les deux grandeurs mesurées sont donc :

- A = PC − Patm = q·[(9/4)·cos²β − 5/4]
- B = PG − PD = (9/4)·q·sin 2β
- B/A = (9/4)·sin 2β / [(9/4)·cos²β − 5/4], fonction de β seul.

Valeurs à 40 km/h (ρ = 1,2 kg/m³, q = 74,1 Pa), générées par `analysis/theory_curves.py` :

| β | A (Pa) | PG (Pa) | PD (Pa) | B (Pa) | B/A |
|---|---|---|---|---|---|
| 0° | 74,1 | −9,3 | −9,3 | 0 | 0 |
| 5° | 72,8 | 5,2 | −23,7 | 28,9 | 0,40 |
| 10° | 69,0 | 19,2 | −37,8 | 57,0 | 0,83 |
| 15° | 62,9 | 32,4 | −50,9 | 83,3 | 1,32 |
| 20° | 54,6 | 44,3 | −62,8 | 107,1 | 1,96 |
| 30° | 32,4 | 62,9 | −81,4 | 144,3 | 4,45 |

Sensibilité autour de 0° : B ≈ 4,5·q·β (β en rad), soit ≈ 5,8 Pa par degré à 40 km/h. A baisse de 7 % à 10° de lacet et de 56 % à 30° : il faut corriger A du lacet pour retrouver q.

Limites de ce modèle (la calibration fait foi) :

- Écoulement réel avec couche limite, nez demi-sphérique prolongé par un cylindre Ø24, Re ≈ 1,8·10⁴ sur le diamètre de la tête : écarts attendus de quelques centièmes de q sur Cp.
- Le blocage amont du vélo et du cycliste décale les coefficients, surtout en lacet.
- Patm de référence : l'air du boîtier n'est pas à la pression statique libre ; l'écart varie avec la vitesse de quelques pascals et n'est pas corrigé par le zéro à l'arrêt.
- ρ = 1,2 est une valeur type ; à calculer à partir de P, T et humidité.

Le retard pneumatique est négligeable (< 0,1 ms pour une ligne de 0,6 m en tuyau de 3 mm de diamètre intérieur avec un orifice de Ø1,3), et les résonances de ligne sont à 140 Hz et plus, au-dessus de la bande utile à 20 Hz.

Ordres de grandeur de q : 40 km/h → ≈ 74 Pa ; 50 km/h → ≈ 118 Pa. Gamme ±500 Pa (limite ≈ 103 km/h en air axial) ; le ±125 Pa saturerait vers 51 km/h.

## Densité de l'air

q = ½ρV², donc ρ est nécessaire : pression atmosphérique absolue + température + idéalement humidité. Prévoir un capteur combiné pression/température (et humidité si possible) ou saisir les valeurs à la main à chaque séance au début. Les SDP810/811 donnent aussi une température (non fiable comme température ambiante, car dans le boîtier).

## Dérive du zéro

Refaire le zéro à l'arrêt, hors vent, avec la tête protégée (sac plastique autour), avant chaque sortie (bouton dédié : moyenne de N échantillons, offset stocké). Sensirion annonce une absence de dérive de zéro sur cette gamme, mais la mécanique de la sonde peut dériver thermiquement.

## Bilan de puissance (étape 2, SCx)

P = SCx·½ρ·V_air³ + Crr·m·g·V_sol·cosθ + m·g·V_sol·sinθ + m·a·V_sol (+ pertes de transmission).

Masse totale ≈ 77 + 9,5 = 86,5 kg + équipement. La projection de V_air sur l'axe du vélo (cos β) et le SCx dépendant du lacet sont à traiter à ce stade.

Fréquence d'échantillonnage visée : 20 Hz (les systèmes commerciaux sont à 10–20 Hz).

## Étalonnage de la vitesse (route, allers-retours)

Les allers-retours suffisent pour la vitesse : le vent longitudinal se compense.

- Route droite, plate, vent quasi nul (tôt le matin ; anémomètre au sol idéalement, < 1 m/s).
- Paliers de vitesse constante (par ex. 15, 25, 35, 45 km/h), au moins 10 s stabilisées par passe. Pas de calibration en accélération.
- Ne pas simplement moyenner les deux sens : un vent résiduel w crée un biais sur q (~w²/V²). Ajuster le modèle √q = k·(V_sol + s·w), s = ±1 selon le sens, par régression : on obtient k et le vent résiduel w de la séance.
- Vitesse sol de référence : capteur de roue avec circonférence mesurée par rollout (mieux que le GPS seul), idéalement croisée avec le GPS.
- Noter pression, température, humidité de la séance.
- Cette calibration se fait avec le montage complet (tube + boîtier), pas la tête seule.

## Étalonnage du lacet (support pivotant à crans)

Sur route on ne connaît pas l'angle réel : il faut l'imposer.

- Support pivotant (`hardware/support_pivotant.svg`) : embase fixe en secteur de disque avec 13 trous d'indexage tous les 5° de −30° à +30° sur un rayon de 60 mm ; bras tournant avec collier de tube ; pivot M5 + écrou à ailettes (tête noyée dans l'embase, rondelle nylon) ; pion d'indexage Ø3 ; fixation vélo par quart de tour.
- La sonde balaie latéralement L·sin β (≈ 200 mm à 30° avec 400 mm, 250 mm avec 500 mm) : à prévoir pour le dégagement.
- Utilisation : montage sur galerie de voiture en porte-à-faux vers l'avant (sortir de l'écoulement perturbé), vitesse constante, vent calme, un cran à la fois, 2–3 vitesses pour vérifier que K_β ne dépend pas de la vitesse. On obtient K_β(β) et Cq(β).
- Limite : sur le vélo, l'écoulement est modifié par le cycliste et le cadre ; le coefficient obtenu sur voiture est une base. Validation sur vélo : à vitesse GPS constante par temps calme, V_air doit retrouver V_sol et le lacet rester proche de zéro.
- Le ventilateur reste utile pour : fuites, zéro, linéarité grossière, symétrie gauche/droite. Il ne remplace pas l'étalonnage sur route.

## Test d'étanchéité (à chaque canal)

Boucher l'extrémité du tuyau et vérifier que la lecture du capteur reste stable dans le temps (une dérive = fuite). Un canal bouché donne une lecture figée. Un temps de réponse anormalement lent = obstruction partielle ou jonction ratée.
