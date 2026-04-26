#include <vector>    
#include <algorithm>  
#include <limits>
#include <chrono>      
#include <fstream> 
#include <filesystem>  
#include <functional>   
#include <iostream>   
#include <numeric>     

#include "modelo.h" 

namespace fs = std::filesystem;
using Clock     = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

// ─────────────────────────────────────────────────────────────────
// Resultado: lo que devuelve el algoritmo al terminar
// ─────────────────────────────────────────────────────────────────

struct Resultado {
    double costo = std::numeric_limits<double>::infinity();
    std::vector<std::vector<std::pair<Avion, int>>> plan;
};

// ─────────────────────────────────────────────────────────────────
// Metricas: contadores internos del algoritmo
// ─────────────────────────────────────────────────────────────────

struct Metricas {
    long long nodos_explorados     = 0;  // cuántos nodos visitó el árbol
    long long soluciones_factibles = 0;  // cuántas veces llegó al caso base
    long long factible_ok          = 0;  // es_factible() devolvió true
    long long factible_fallo       = 0;  // es_factible() devolvió false (podas)
};

// ─────────────────────────────────────────────────────────────────
// guardar_csv: reemplaza a matplotlib
// guarda los datos de cada gráfico en un archivo .csv
// "static" = solo visible en este .cpp, evita conflictos al enlazar
// ─────────────────────────────────────────────────────────────────

static void guardar_csv(const std::string& ruta,
                        const std::string& col_x, const std::string& col_y,
                        const std::vector<double>& xs, const std::vector<double>& ys)
{
    std::ofstream f(ruta);                        // abre/crea el archivo
    f << col_x << "," << col_y << "\n";           // cabecera: "tiempo_s,nodos"
    for (size_t i = 0; i < xs.size(); ++i)
        f << xs[i] << "," << ys[i] << "\n";       // un par de datos por línea
}

// ─────────────────────────────────────────────────────────────────
// Función principal
// devuelve pair<Resultado, Metricas> → equivale a: return mejor_estado, metricas
// ─────────────────────────────────────────────────────────────────

std::pair<Resultado, Metricas>
backtracking_cronologico(const ProblemaALP& problema,
                         int tiempo_limite_seg       = 1200,  // 20 min por defecto
                         const std::string& nombre_caso = "caso",
                         int num_pistas              = 3)
{
    // pistas[i] = aviones asignados a la pista i con su tiempo
    // equivale a: pistas = [[] for _ in range(num_pistas)]
    std::vector<std::vector<std::pair<Avion, int>>> pistas(num_pistas);

    Resultado mejor;   // costo = infinito al inicio
    Metricas  stats;   // todos los contadores en 0

    // flag de tiempo límite — reemplaza la excepción TimeoutError de Python
    bool timeout = false;

    // copia local para poder ordenar sin modificar el problema original
    std::vector<Avion> aviones = problema.aviones;

    // equivale a: sorted(problema.aviones, key=lambda x: x.p)
    std::sort(aviones.begin(), aviones.end(),
              [](const Avion& a, const Avion& b){ return a.p < b.p; });

    int total  = static_cast<int>(aviones.size());
    TimePoint t0 = Clock::now();  // instante de inicio del reloj

    // ── series de datos para los CSVs (equivalen a las listas de Python) ──
    std::vector<double> tiempos_nodos, serie_nodos;
    std::vector<double> tiempos_costos, serie_costos;
    std::vector<double> tiempos_poda,   serie_poda;
    std::vector<double> nodos_costos,   costos_por_nodo;

    // histograma: cuántos nodos se exploraron en cada nivel del árbol
    std::vector<double> nodos_por_nivel(total + 1, 0.0);

    // ─────────────────────────────────────────────────────────────
    // Lambda recursiva — equivale a def resolver(idx): en Python
    //
    // std::function<void(int)> permite que la lambda se llame a sí misma
    // [&] captura todo por referencia → equivale al nonlocal de Python
    // ─────────────────────────────────────────────────────────────

    std::function<void(int)> resolver = [&](int idx) {

        if (timeout) return;  // tiempo agotado → corta sin hacer nada

        // ── contadores y registro de tiempo ──
        stats.nodos_explorados++;
        nodos_por_nivel[idx]++;

        // tiempo transcurrido en segundos
        // equivale a: tiempo_actual = time.time() - start_time
        double seg = std::chrono::duration<double>(Clock::now() - t0).count();

        // .push_back() = .append() de Python
        tiempos_nodos.push_back(seg);
        serie_nodos.push_back(static_cast<double>(stats.nodos_explorados));

        // tasa de poda = proporción de veces que es_factible devolvió false
        long long total_checks = stats.factible_ok + stats.factible_fallo;
        if (total_checks > 0) {
            tiempos_poda.push_back(seg);
            serie_poda.push_back(
                static_cast<double>(stats.factible_fallo) / total_checks);
        }

        // equivale al: if tiempo_actual > tiempo_limite_seg: raise TimeoutError
        if (seg > tiempo_limite_seg) { timeout = true; return; }

        // ── CASO BASE: se asignaron todos los aviones ──
        if (idx == total) {
            stats.soluciones_factibles++;

            // suma el costo de cada avión en su tiempo asignado
            double costo_total = 0.0;
            for (auto& pista : pistas)
                for (auto& [av, t] : pista)   // [av,t] = desestructuración del pair
                    costo_total += av.calcular_f_tk(t);

            if (costo_total < mejor.costo) {
                mejor.costo = costo_total;

                // copia profunda automática (vector de structs por valor)
                // equivale al copy.deepcopy(pistas) de Python
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
            for (int t = av.e; t <= av.l && !timeout; ++t) {

                if (problema.es_factible(av, t, pistas[p])) {
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

    if (timeout)
        std::cout << "\nTiempo limite alcanzado\n";
    else
        std::cout << "\n--- El algoritmo termino de explorar todo el espacio.\n";

    // ─────────────────────────────────────────────────────────────
    // Persistencia de resultados
    // ─────────────────────────────────────────────────────────────

    std::string base = "ejecuciones/backtracking/" + nombre_caso;

    // equivale a: os.makedirs(ruta_base, exist_ok=True)
    fs::create_directories(base);

    // un CSV por cada gráfico que antes hacía matplotlib
    guardar_csv(base + "/nodos_vs_tiempo.csv",
                "tiempo_s", "nodos", tiempos_nodos, serie_nodos);

    if (!tiempos_costos.empty())
        guardar_csv(base + "/costo_vs_tiempo.csv",
                    "tiempo_s", "costo", tiempos_costos, serie_costos);

    if (!tiempos_poda.empty())
        guardar_csv(base + "/poda_vs_tiempo.csv",
                    "tiempo_s", "tasa_poda", tiempos_poda, serie_poda);

    if (!nodos_costos.empty())
        guardar_csv(base + "/costo_vs_nodos.csv",
                    "nodos", "costo", nodos_costos, costos_por_nodo);

    {
        // rellena 0, 1, 2, ..., total
        // equivale al range(len(nodos_por_nivel)) del bar chart de Python
        std::vector<double> niveles(total + 1);
        std::iota(niveles.begin(), niveles.end(), 0.0);
        guardar_csv(base + "/nodos_por_nivel.csv",
                    "nivel", "nodos", niveles, nodos_por_nivel);
    }

    // metricas.txt — mismo contenido que en Python
    {
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
                    f << "  [T=" << t << "] Avion " << av.id << "\n";
            }
        }

        f << "\nMETRICAS\n";
        f << "Nodos explorados: "     << stats.nodos_explorados    << "\n";
        f << "Soluciones factibles: " << stats.soluciones_factibles << "\n";
        f << "es_factible = True: "   << stats.factible_ok         << "\n";
        f << "es_factible = False: "  << stats.factible_fallo      << "\n";
    }

    // equivale al: return mejor_estado, metricas
    return {mejor, stats};
}