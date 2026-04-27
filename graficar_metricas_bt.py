import argparse
import csv
from pathlib import Path

import matplotlib

# Evita bloqueos por backends interactivos en consola.
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def leer_columnas(path_csv: Path, col_x: str, col_y: str):
    xs = []
    ys = []

    with path_csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames or []
        if col_x not in fieldnames or col_y not in fieldnames:
            raise ValueError(
                f"CSV invalido en {path_csv}. Se esperaban columnas '{col_x}' y '{col_y}'."
            )
        for row in reader:
            xs.append(float(row[col_x]))
            ys.append(float(row[col_y]))

    return xs, ys


def graficar(path_csv: Path, col_x: str, col_y: str, titulo: str, out_png: Path):
    xs, ys = leer_columnas(path_csv, col_x, col_y)
    if not xs:
        print(f"[SKIP] {path_csv.name}: no hay datos.")
        return

    plt.figure(figsize=(10, 5))
    plt.plot(xs, ys, linewidth=1.5)
    plt.title(titulo)
    plt.xlabel(col_x)
    plt.ylabel(col_y)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(out_png, dpi=150)
    plt.close()
    print(f"[OK] Grafica guardada: {out_png}")


def obtener_columnas_csv(path_csv: Path):
    with path_csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames or []
    if len(fieldnames) < 2:
        raise ValueError(
            f"CSV invalido en {path_csv}. Debe tener al menos 2 columnas."
        )
    return fieldnames[0], fieldnames[1]


def generar_grafico_para_csv(path_csv: Path):
    try:
        col_x, col_y = obtener_columnas_csv(path_csv)
        out_png = path_csv.with_suffix(".png")
        titulo = f"{col_y} vs {col_x} ({path_csv.parent.name})"
        graficar(path_csv, col_x, col_y, titulo, out_png)
    except Exception as e:
        print(f"[SKIP] {path_csv}: {e}")


def main():
    parser = argparse.ArgumentParser(
        description="Genera graficos PNG para todos los CSV en ejecuciones."
    )
    parser.add_argument(
        "--base",
        type=Path,
        default=Path("ejecuciones"),
        help="Directorio base donde buscar CSV (por defecto: ejecuciones).",
    )
    parser.add_argument(
        "--csv",
        type=Path,
        default=None,
        help="Si lo indicas, genera solo para ese CSV.",
    )
    args = parser.parse_args()

    if args.csv is not None:
        if not args.csv.exists():
            raise FileNotFoundError(f"No existe el CSV: {args.csv}")
        generar_grafico_para_csv(args.csv)
        print("Listo.")
        return

    if not args.base.exists():
        raise FileNotFoundError(f"No existe el directorio base: {args.base}")

    csvs = sorted(args.base.rglob("*.csv"))
    if not csvs:
        print(f"No se encontraron CSV en: {args.base}")
        return

    print(f"Se encontraron {len(csvs)} CSV. Generando graficos...")
    for path_csv in csvs:
        generar_grafico_para_csv(path_csv)

    print("Listo.")


if __name__ == "__main__":
    main()
