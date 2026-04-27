import argparse
import csv
import time
from pathlib import Path

import matplotlib

# Backend no interactivo: evita bloqueos y mejora estabilidad en consola.
matplotlib.use("Agg")
import matplotlib.pyplot as plt


TARGET_FILES = {"nodos_vs_tiempo.csv", "poda_vs_tiempo.csv"}


def contar_filas_datos(path_csv: Path):
    with path_csv.open("r", encoding="utf-8", newline="") as f:
        total = sum(1 for _ in f) - 1  # sin cabecera
    return max(total, 0)


def leer_xy_muestreado(path_csv: Path, max_points: int):
    xs = []
    ys = []
    total_filas = contar_filas_datos(path_csv)
    if total_filas <= 0:
        raise ValueError("CSV sin datos.")

    paso = max(1, total_filas // max_points)

    with path_csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        cols = reader.fieldnames or []
        if len(cols) < 2:
            raise ValueError("El CSV debe tener al menos 2 columnas.")

        x_name, y_name = cols[0], cols[1]
        for idx, row in enumerate(reader):
            # Muestreo determinista por salto para controlar memoria/tiempo.
            if idx % paso != 0 and idx != total_filas - 1:
                continue
            xs.append(float(row[x_name]))
            ys.append(float(row[y_name]))

    if not xs:
        raise ValueError("CSV sin datos.")

    return xs, ys, x_name, y_name, total_filas


def graficar(path_csv: Path, force: bool, max_points: int):
    out_png = path_csv.with_suffix(".png")
    if out_png.exists() and not force:
        return "skip", f"PNG ya existe: {out_png}"

    xs, ys, x_name, y_name, total_filas = leer_xy_muestreado(path_csv, max_points)

    plt.figure(figsize=(10, 5))
    plt.plot(xs, ys, linewidth=1.4, color="tab:blue")
    plt.title(f"{y_name} vs {x_name} ({path_csv.parent.name})")
    plt.xlabel(x_name)
    plt.ylabel(y_name)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(out_png, dpi=130)
    plt.close()

    return (
        "ok",
        f"Guardado: {out_png} | puntos usados={len(xs)} de {total_filas}",
    )


def main():
    parser = argparse.ArgumentParser(
        description="Grafica solo nodos_vs_tiempo.csv y poda_vs_tiempo.csv."
    )
    parser.add_argument(
        "--base",
        type=Path,
        default=Path("ejecuciones"),
        help="Directorio base donde buscar CSV (default: ejecuciones).",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Re-genera PNG aunque ya exista.",
    )
    parser.add_argument(
        "--max-points",
        type=int,
        default=200_000,
        help="Maximo de puntos por grafico (default: 200000).",
    )
    args = parser.parse_args()
    if args.max_points <= 0:
        raise ValueError("--max-points debe ser mayor que 0.")

    if not args.base.exists():
        raise FileNotFoundError(f"No existe el directorio base: {args.base}")

    candidatos = sorted(
        p for p in args.base.rglob("*.csv") if p.name.lower() in TARGET_FILES
    )

    if not candidatos:
        print(f"No se encontraron nodos/poda CSV en: {args.base}")
        return

    total = len(candidatos)
    ok = 0
    skip = 0
    fail = 0
    t0 = time.perf_counter()

    print(f"[Inicio] Archivos objetivo: {total}")
    for i, path_csv in enumerate(candidatos, start=1):
        ti = time.perf_counter()
        progreso = f"[{i}/{total}]"
        print(f"{progreso} Procesando: {path_csv}")

        try:
            estado, msg = graficar(
                path_csv, force=args.force, max_points=args.max_points
            )
            dt = time.perf_counter() - ti
            if estado == "ok":
                ok += 1
                print(f"{progreso} [OK] {msg} ({dt:.2f}s)")
            else:
                skip += 1
                print(f"{progreso} [SKIP] {msg} ({dt:.2f}s)")
        except Exception as e:
            fail += 1
            dt = time.perf_counter() - ti
            print(f"{progreso} [ERROR] {path_csv} -> {e} ({dt:.2f}s)")

        transcurrido = time.perf_counter() - t0
        promedio = transcurrido / i
        restantes = total - i
        eta = promedio * restantes
        print(
            f"{progreso} Avance: {i/total:.0%} | OK={ok} SKIP={skip} ERROR={fail} | ETA~{eta:.1f}s"
        )

    total_t = time.perf_counter() - t0
    print("\n[Fin] Resumen")
    print(f"  OK:    {ok}")
    print(f"  SKIP:  {skip}")
    print(f"  ERROR: {fail}")
    print(f"  Tiempo total: {total_t:.2f}s")


if __name__ == "__main__":
    main()
