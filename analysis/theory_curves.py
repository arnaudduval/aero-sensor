#!/usr/bin/env python3
"""Courbes théoriques de la sonde 3 trous (sphère en écoulement potentiel).

Orifice central sur l'axe, deux orifices latéraux dont la normale fait ±45° avec l'axe.
    Cp(psi) = 9/4 cos^2(psi) - 5/4       (psi = angle normale / direction amont)
    A = PC - Patm = q Cp(beta)
    B = PG - PD   = q [Cp(45-beta) - Cp(45+beta)] = 9/4 q sin(2 beta)

Usage : python3 theory_curves.py [--kmh 40] [--rho 1.2] [--png courbes.png]
Sert de référence théorique à superposer aux courbes d'étalonnage mesurées ;
seule la calibration sur le support pivotant fait foi.
"""
import argparse
import math


def cp(psi_rad):
    return 2.25 * math.cos(psi_rad) ** 2 - 1.25


def ports(beta_deg, q):
    b = math.radians(beta_deg)
    pc = q * cp(b)
    pg = q * cp(math.radians(45.0) - b)
    pd = q * cp(math.radians(45.0) + b)
    return pc, pg, pd


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--kmh", type=float, default=40.0)
    ap.add_argument("--rho", type=float, default=1.2)
    ap.add_argument("--png", default=None, help="fichier PNG (nécessite matplotlib)")
    a = ap.parse_args()

    v = a.kmh / 3.6
    q = 0.5 * a.rho * v * v
    print(f"V = {a.kmh:.1f} km/h, rho = {a.rho} kg/m3, q = {q:.1f} Pa")
    print(" beta |   A=PC-Patm |      PG |      PD |  B=PG-PD |  B/A")
    betas = [0, 5, 10, 15, 20, 25, 30]
    for beta in betas:
        pc, pg, pd = ports(beta, q)
        ratio = (pg - pd) / pc if pc else 0.0
        print(f"{beta:4d}  | {pc:10.1f} | {pg:7.1f} | {pd:7.1f} | {pg - pd:8.1f} | {ratio:5.2f}")

    if a.png:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt

        xs = [i * 0.5 for i in range(-60, 61)]
        data = [ports(x, q) for x in xs]
        fig, ax = plt.subplots(1, 2, figsize=(10, 4))
        ax[0].plot(xs, [d[0] for d in data], label="PC - Patm (A)")
        ax[0].plot(xs, [d[1] - d[2] for d in data], label="PG - PD (B)")
        ax[0].set_xlabel("lacet (deg)")
        ax[0].set_ylabel("Pa")
        ax[0].set_title(f"{a.kmh:.0f} km/h")
        ax[0].legend()
        ax[0].grid(alpha=0.3)
        xr = [x for x in xs if abs(x) <= 30 and x != 0]
        ax[1].plot(xr, [(ports(x, q)[1] - ports(x, q)[2]) / ports(x, q)[0] for x in xr])
        ax[1].set_xlabel("lacet (deg)")
        ax[1].set_ylabel("B / A")
        ax[1].set_title("rapport indépendant de q")
        ax[1].grid(alpha=0.3)
        fig.tight_layout()
        fig.savefig(a.png, dpi=130)
        print("écrit", a.png)


if __name__ == "__main__":
    main()
