# Analyse

Scripts existants :

- `theory_curves.py` : courbes théoriques (sphère potentielle) de A = PC−Patm, B = PG−PD et B/A en fonction du lacet ; `python3 theory_curves.py --kmh 40 --png courbes.png`.

Scripts prévus (voir `docs/05-firmware.md`, section post-traitement) :

- `read_log.py` : décodage du log binaire de l'ESP32 (format à définir avec le firmware) et alignement avec un éventuel fichier FIT.
- `fit_speed_calibration.py` : régression √q = k·(V_sol + s·w) sur des fenêtres stables ≥ 10 s, par sens (s = ±1).
- `fit_yaw_curves.py` : courbes (PG−PD)/(PC−Patm)(β) et Cq(β) à partir des passes sur le support pivotant.
- `scx_estimate.py` : bilan de puissance et SCx, après les deux calibrations.
