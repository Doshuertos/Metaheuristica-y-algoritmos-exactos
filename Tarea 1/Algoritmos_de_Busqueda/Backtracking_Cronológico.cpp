#include <vector>    
#include <algorithm>  
#include <limits>
#include <chrono>      
#include <cmath>
#include <fstream> 
#include <filesystem>  
#include <functional>   
#include <iostream>   
#include <numeric>     
#include <string>

#include "../Clase/Avion.h"
#include "../Utilidades/Factivilidad.h"
#include "Backtracking_Cronológico.h"

namespace fs = std::filesystem;
using Clock     = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

// Muestreo para CSV: mas denso que antes (mejor para analisis) sin volver a
// una fila por nodo (eso generaba archivos enormes y lentos).
// Cota aproximada: ~600/intervalo muestras por tiempo + nodos/salto por eje nodos.
namespace {
constexpr double kMuestraMinIntervaloSeg = 0.05;
constexpr long long kMuestraSaltoNodos   = 50'000;
}

static void guardar_csv(const std::string& ruta,
                        const std::string& col_x, const std::string& col_y,
                        const std::vector<double>& xs, const std::vector<double>& ys)
{
    std::ofstream f(ruta);                        // abre/crea el archivo
    f << col_x << "," << col_y << "\n";           // cabecera: "tiempo_s,nodos"
    for (size_t i = 0; i < xs.size(); ++i)
        f << xs[i] << "," << ys[i] << "\n";       // un par de datos por línea
}


std::pair<Resultado, Metricas>
backtracking_cronologico(const std::vector<Avion>& aviones_entrada,
                         int tiempo_limite_seg,
                         const std::string& nombre_caso,
                         int num_pistas)
{
    std::vector<std::vector<std::pair<Avion, int>>> pistas(num_pistas);

    Resultado mejor;   // costo = infinito al inicio
    Metricas  stats;   // todos los contadores en 0

    bool timeout = false;
    int ultimo_log_seg = -1;
    int ultimo_mejor_seg = 0;

    bool primera_muestra_csv = true;
    double ult_muestra_seg = 0.0;
    long long ult_muestra_nodo = 0;

    // copia local para poder ordenar sin modificar el problema original
    std::vector<Avion> aviones = aviones_entrada;

    std::sort(aviones.begin(), aviones.end(),
              [](const Avion& a, const Avion& b){ return a.Pk < b.Pk; });

    int total  = static_cast<int>(aviones.size());
    TimePoint t0 = Clock::now();  // instante de inicio del reloj

    // ── series de datos para los CSVs ──
    std::vector<double> tiempos_nodos, serie_nodos;
    std::vector<double> tiempos_costos, serie_costos;
    std::vector<double> tiempos_poda,   serie_poda;
    std::vector<double> nodos_costos,   costos_por_nodo;

    // histograma: cuántos nodos se exploraron en cada nivel del árbol
    std::vector<double> nodos_por_nivel(total + 1, 0.0);

    std::function<void(int)> resolver = [&](int idx) {

        if (timeout) return;  // tiempo agotado → corta sin hacer nada

        // ── contadores y registro de tiempo ──
        stats.nodos_explorados++;
        nodos_por_nivel[idx]++;

        // tiempo transcurrido en segundos
        double seg = std::chrono::duration<double>(Clock::now() - t0).count();
        int seg_int = static_cast<int>(seg);

        // tasa de poda = proporción de veces que es_factible devolvió false
        long long total_checks = stats.factible_ok + stats.factible_fallo;

        bool tomar_muestra = primera_muestra_csv;
        if (!tomar_muestra) {
            if (seg - ult_muestra_seg >= kMuestraMinIntervaloSeg)
                tomar_muestra = true;
            if (stats.nodos_explorados - ult_muestra_nodo >= kMuestraSaltoNodos)
                tomar_muestra = true;
        }
        if (tomar_muestra) {
            primera_muestra_csv = false;
            ult_muestra_seg = seg;
            ult_muestra_nodo = stats.nodos_explorados;
            tiempos_nodos.push_back(seg);
            serie_nodos.push_back(static_cast<double>(stats.nodos_explorados));
            if (total_checks > 0) {
                tiempos_poda.push_back(seg);
                serie_poda.push_back(
                    static_cast<double>(stats.factible_fallo) / total_checks);
            }
        }

        if (seg >= tiempo_limite_seg) { timeout = true; return; }

        // Log de progreso cada 10 segundos para saber que el algoritmo sigue activo.
        if (seg_int / 10 > ultimo_log_seg / 10) {
            ultimo_log_seg = seg_int;
            int sin_mejora = seg_int - ultimo_mejor_seg;
            std::cout << "    Progreso -> T=" << seg_int
                      << "s, nodos=" << stats.nodos_explorados
                      << ", mejor=";
            if (std::isfinite(mejor.costo)) std::cout << mejor.costo;
            else std::cout << "INF";
            std::cout << ", sin mejora=" << sin_mejora << "s\n";
        }

        // ── CASO BASE: se asignaron todos los aviones ──
        if (idx == total) {
            stats.soluciones_factibles++;

            // suma el costo de cada avión en su tiempo asignado
            double costo_total = 0.0;
            for (auto& pista : pistas)
                for (auto& [av, t] : pista)   // [av,t] = desestructuración del pair
                    costo_total += av.Calcular_Penalizaciones(t);

            if (costo_total < mejor.costo) {
                mejor.costo = costo_total;
                ultimo_mejor_seg = seg_int;

                mejor.plan = pistas;

                tiempos_costos.push_back(seg);
                serie_costos.push_back(costo_total);
                nodos_costos.push_back(static_cast<double>(stats.nodos_explorados));
                costos_por_nodo.push_back(costo_total);

                std::cout << "    Mejora hallada! Costo: " << costo_total
                          << " (T=" << static_cast<int>(seg) << "s)\n";
            }
            return;
        }

        // ── EXPANSIÓN: prueba cada combinación (pista, tiempo) ──
        const Avion& av = aviones[idx];

        // && !timeout en los for evita seguir iterando si se agotó el tiempo
        for (int p = 0; p < num_pistas && !timeout; ++p) {
            for (int t = av.Ek; t <= av.Lk && !timeout; ++t) {

                if (es_factible(av, t, pistas[p])) {
                    stats.factible_ok++;

                    pistas[p].push_back({av, t});  // asigna → pistas[p].append(...)
                    resolver(idx + 1);              // llama al siguiente avión
                    pistas[p].pop_back();           // deshace → pistas[p].pop()

                } else {
                    stats.factible_fallo++;
                }
            }
        }
    };  // fin lambda

    // ── arranque ──
    std::cout << "Iniciando Backtracking (Pistas=" << num_pistas
              << ", Limite: " << tiempo_limite_seg / 60.0 << " min)...\n";

    resolver(0);  // empieza desde el avión 0

    {
        double seg_fin =
            std::chrono::duration<double>(Clock::now() - t0).count();
        if (!tiempos_nodos.empty()) {
            bool distinto =
                (std::abs(tiempos_nodos.back() - seg_fin) > 1e-9) ||
                (serie_nodos.back() !=
                 static_cast<double>(stats.nodos_explorados));
            if (distinto) {
                tiempos_nodos.push_back(seg_fin);
                serie_nodos.push_back(
                    static_cast<double>(stats.nodos_explorados));
                long long tc = stats.factible_ok + stats.factible_fallo;
                if (tc > 0) {
                    tiempos_poda.push_back(seg_fin);
                    serie_poda.push_back(
                        static_cast<double>(stats.factible_fallo) / tc);
                }
            }
        }
    }

    double tiempo_total_seg = std::chrono::duration<double>(Clock::now() - t0).count();
    if (timeout)
        std::cout << "\nTiempo limite alcanzado (Tiempo total: "
                  << tiempo_total_seg << " s)\n";
    else
        std::cout << "\n--- El algoritmo termino de explorar todo el espacio "
                  << "(Tiempo total: " << tiempo_total_seg << " s).\n";

    std::cout << "Guardando metricas y resultados en disco...\n";

    // ─────────────────────────────────────────────────────────────
    // Persistencia de resultados
    // ─────────────────────────────────────────────────────────────

    std::string base = "ejecuciones/backtracking/" + nombre_caso;

    std::cout << "  [1/7] Preparando carpeta de salida...\n";
    fs::create_directories(base);

    std::cout << "  [2/7] Guardando nodos_vs_tiempo.csv...\n";
    guardar_csv(base + "/nodos_vs_tiempo.csv",
                "tiempo_s", "nodos", tiempos_nodos, serie_nodos);

    if (!tiempos_costos.empty()) {
        std::cout << "  [3/7] Guardando costo_vs_tiempo.csv...\n";
        guardar_csv(base + "/costo_vs_tiempo.csv",
                    "tiempo_s", "costo", tiempos_costos, serie_costos);
    } else {
        std::cout << "  [3/7] Saltado costo_vs_tiempo.csv (sin mejoras registradas).\n";
    }

    if (!tiempos_poda.empty()) {
        std::cout << "  [4/7] Guardando poda_vs_tiempo.csv...\n";
        guardar_csv(base + "/poda_vs_tiempo.csv",
                    "tiempo_s", "tasa_poda", tiempos_poda, serie_poda);
    } else {
        std::cout << "  [4/7] Saltado poda_vs_tiempo.csv (sin datos de poda).\n";
    }

    if (!nodos_costos.empty()) {
        std::cout << "  [5/7] Guardando costo_vs_nodos.csv...\n";
        guardar_csv(base + "/costo_vs_nodos.csv",
                    "nodos", "costo", nodos_costos, costos_por_nodo);
    } else {
        std::cout << "  [5/7] Saltado costo_vs_nodos.csv (sin mejoras registradas).\n";
    }

    {
        std::cout << "  [6/7] Guardando nodos_por_nivel.csv...\n";
        // rellena 0, 1, 2, ..., total
        std::vector<double> niveles(total + 1);
        std::iota(niveles.begin(), niveles.end(), 0.0);
        guardar_csv(base + "/nodos_por_nivel.csv",
                    "nivel", "nodos", niveles, nodos_por_nivel);
    }

    // metricas.txt 
        std::cout << "  [7/7] Guardando metricas.txt...\n";
        std::ofstream f(base + "/metricas.txt");
        f << "BACKTRACKING\n----------------------------------------\n";
        f << "Mejor Costo: " << mejor.costo << "\n\n";

        if (!mejor.plan.empty()) {
            for (int i = 0; i < (int)mejor.plan.size(); ++i) {
                auto pista = mejor.plan[i];

                // ordena por tiempo antes de escribir
                std::sort(pista.begin(), pista.end(),
                          [](auto& a, auto& b){ return a.second < b.second; });

                f << "Pista " << (i + 1) << ":\n";
                for (auto& [av, t] : pista)
                    f << "  [T=" << t << "] Avion " << av.id_avion << "\n";
            }
        }

        f << "\nMETRICAS\n";
        f << "Nodos explorados: "     << stats.nodos_explorados    << "\n";
        f << "Soluciones factibles: " << stats.soluciones_factibles << "\n";
        f << "es_factible = True: "   << stats.factible_ok         << "\n";
        f << "es_factible = False: "  << stats.factible_fallo      << "\n";
    std::cout << "Persistencia completada.\n";
    return {mejor, stats};
}