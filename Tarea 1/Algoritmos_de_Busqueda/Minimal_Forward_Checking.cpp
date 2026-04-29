#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <fstream>
#include <string>

#include "Lector.h"
#include "Avion.h"

using namespace std;

struct Opcion {
    int pista;
    int tiempo;
    double costo_individual;
};

// -------- METRICAS GLOBALES --------
double mejor_costo = numeric_limits<double>::infinity();
long long nodos_explorados = 0;
long long podas_mfc = 0;
long long podas_costo = 0;
long long chequeos_futuros = 0;
long long opciones_probadas = 0;

vector<double> registro_tiempos;
vector<double> registro_costos;
vector<long long> nodos_por_nivel;

int total_aviones;
vector<Avion> lista_aviones;
auto hora_inicio_algoritmo = chrono::high_resolution_clock::now();
const int TIEMPO_LIMITE_SEG = 600; // 10 minutos
// false: separacion solo cuando comparten pista (modelo original de tu codigo)
// true : separacion global entre cualquier par de aviones
const bool SEPARACION_GLOBAL = false;
vector<Opcion> asignaciones;
vector<Opcion> mejor_asignacion;
bool tiempo_agotado = false;

bool opciones_compatibles(const Avion& av_i, const Opcion& asig,
                          const Avion& av_j, const Opcion& opt_j) {
    if (!SEPARACION_GLOBAL && asig.pista != opt_j.pista) {
        return true;
    }
    if (asig.tiempo < opt_j.tiempo) {
        return opt_j.tiempo >= asig.tiempo + av_i.Vector_Separacion_Tau[av_j.id_avion - 1];
    }
    return asig.tiempo >= opt_j.tiempo + av_j.Vector_Separacion_Tau[av_i.id_avion - 1];
}

bool consistente_con_pasado(int idx_actual, const Opcion& asig_actual) {
    const Avion& av_actual = lista_aviones[idx_actual];
    for (int i = 0; i < idx_actual; ++i) {
        if (!opciones_compatibles(lista_aviones[i], asignaciones[i], av_actual, asig_actual)) {
            return false;
        }
    }
    return true;
}

bool validar_solucion(const vector<Opcion>& sol, int p_count, double& costo_recalculado) {
    costo_recalculado = 0.0;

    for (int i = 0; i < total_aviones; ++i) {
        if (sol[i].pista < 0 || sol[i].pista >= p_count) return false;
        if (sol[i].tiempo < lista_aviones[i].Ek || sol[i].tiempo > lista_aviones[i].Lk) return false;
        costo_recalculado += lista_aviones[i].Calcular_Penalizaciones(sol[i].tiempo);
    }

    for (int i = 0; i < total_aviones; ++i) {
        for (int j = i + 1; j < total_aviones; ++j) {
            if (!SEPARACION_GLOBAL && sol[i].pista != sol[j].pista) continue;

            if (sol[i].tiempo < sol[j].tiempo) {
                if (sol[j].tiempo < sol[i].tiempo + lista_aviones[i].Vector_Separacion_Tau[lista_aviones[j].id_avion - 1]) return false;
            } else if (sol[j].tiempo < sol[i].tiempo) {
                if (sol[i].tiempo < sol[j].tiempo + lista_aviones[j].Vector_Separacion_Tau[lista_aviones[i].id_avion - 1]) return false;
            } else {
                return false;
            }
        }
    }
    return true;
}

// MFC mejorado: para cada avion futuro j, busca una opcion que sea compatible
// con TODO el prefijo actual (indices 0..idx_actual inclusive).
// Ademas, para evitar el "falso 0", verifica que la opcion encontrada para j
// tambien sea compatible con la mejor opcion encontrada para cada avion futuro
// anterior k (idx_actual < k < j). Esto convierte MFC en un FC parcial entre
// pares de aviones futuros, capturando conflictos entre pistas.
bool verificar_futuro_minimo(int idx_actual,
                             const vector<vector<Opcion>>& dominios,
                             vector<Opcion>& mejor_opcion_futura) {
    if (tiempo_agotado) return false;
    // mejor_opcion_futura[j] = la opcion elegida provisionalmente para el avion j
    // (la de menor costo compatible con el prefijo)
    mejor_opcion_futura.assign(total_aviones, {-1, -1, 0.0});

    for (int j = idx_actual + 1; j < total_aviones; ++j) {
        if (tiempo_agotado) return false;
        chequeos_futuros++;
        const Avion& av_j = lista_aviones[j];
        bool existe_opcion_valida = false;

        for (const auto& opt_j : dominios[j]) {
            if (tiempo_agotado) return false;
            opciones_probadas++;

            // 1) Compatible con todo el prefijo ya asignado (0..idx_actual)
            bool ok = true;
            for (int i = 0; i <= idx_actual; ++i) {
                if (!opciones_compatibles(lista_aviones[i], asignaciones[i], av_j, opt_j)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) continue;

            // 2) Compatible con las opciones provisionales de los aviones futuros
            //    anteriores a j (idx_actual < k < j) que ya fueron elegidas.
            for (int k = idx_actual + 1; k < j; ++k) {
                if (mejor_opcion_futura[k].pista == -1) continue; // no asignado aun
                if (!opciones_compatibles(lista_aviones[k], mejor_opcion_futura[k], av_j, opt_j)) {
                    ok = false;
                    break;
                }
            }
            if (!ok) continue;

            // Opcion valida encontrada
            mejor_opcion_futura[j] = opt_j;
            existe_opcion_valida = true;
            break; // Minimal FC: primer valido alcanza
        }

        if (!existe_opcion_valida) {
            podas_mfc++;
            return false;
        }
    }
    return true;
}

void minimal_forward_checking(int idx, double costo_acumulado,
                              const vector<vector<Opcion>>& dominios, int p_actual) {
    if (tiempo_agotado) return;

    auto ahora = chrono::high_resolution_clock::now();
    if (chrono::duration_cast<chrono::seconds>(ahora - hora_inicio_algoritmo).count() > TIEMPO_LIMITE_SEG) {
        tiempo_agotado = true;
        return;
    }

    nodos_explorados++;
    nodos_por_nivel[idx]++;

    if (idx == total_aviones) {
        if (costo_acumulado < mejor_costo) {
            mejor_costo = costo_acumulado;
            mejor_asignacion = asignaciones;
            auto ahora = chrono::high_resolution_clock::now();
            double t = chrono::duration_cast<chrono::milliseconds>(ahora - hora_inicio_algoritmo).count() / 1000.0;
            registro_tiempos.push_back(t);
            registro_costos.push_back(mejor_costo);
            cout << "  [LOG MFC] Nuevo optimo: " << mejor_costo << " en " << t << "s" << endl;
        }
        return;
    }

    if (costo_acumulado >= mejor_costo) {
        podas_costo++;
        return;
    }

    for (const auto& opt : dominios[idx]) {
        if (tiempo_agotado) return;
        double nuevo_costo = costo_acumulado + opt.costo_individual;
        if (nuevo_costo >= mejor_costo) {
            podas_costo++;
            continue;
        }

        if (!consistente_con_pasado(idx, opt)) {
            continue;
        }

        asignaciones[idx] = opt;

        vector<Opcion> mejor_opcion_futura;
        if (verificar_futuro_minimo(idx, dominios, mejor_opcion_futura)) {
            minimal_forward_checking(idx + 1, nuevo_costo, dominios, p_actual);
        }
    }
}

int main(int argc, char* argv[]) {
    string archivo = "Test_Case/case1.txt";
    if (argc >= 2) {
        archivo = argv[1];
    }

    lista_aviones.clear();
    total_aviones = 0;
    cargarArchivo(archivo, lista_aviones, total_aviones);
    if (lista_aviones.empty()) return 1;
    cout << "[INFO] Caso cargado: " << archivo << " | Aviones: " << total_aviones << endl;

    sort(lista_aviones.begin(), lista_aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    ofstream file_res("metricas_mfc.csv");
    file_res << "Pistas,Mejor_Costo,Nodos,Podas_MFC,Podas_BB,Chequeos_Futuros,Opciones_Probadas" << endl;

    for (int p_count = 1; p_count <= 3; ++p_count) {
        mejor_costo = numeric_limits<double>::infinity();
        nodos_explorados = 0;
        podas_mfc = 0;
        podas_costo = 0;
        chequeos_futuros = 0;
        opciones_probadas = 0;
        nodos_por_nivel.assign(total_aviones + 1, 0);
        registro_tiempos.clear();
        registro_costos.clear();
        asignaciones.assign(total_aviones, {-1, -1, 0.0});
        mejor_asignacion.assign(total_aviones, {-1, -1, 0.0});
        tiempo_agotado = false;

        vector<vector<Opcion>> dominios_ini(total_aviones);
        for (int i = 0; i < total_aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = lista_aviones[i].Ek; t <= lista_aviones[i].Lk; ++t) {
                    dominios_ini[i].push_back({p, t, lista_aviones[i].Calcular_Penalizaciones(t)});
                }
            }
            sort(dominios_ini[i].begin(), dominios_ini[i].end(), [](const Opcion& a, const Opcion& b) {
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> TEST MFC: " << p_count << " PISTAS" << endl;
        hora_inicio_algoritmo = chrono::high_resolution_clock::now();
        minimal_forward_checking(0, 0.0, dominios_ini, p_count);

        double costo_recalculado = 0.0;
        bool factible = false;
        if (mejor_costo < numeric_limits<double>::infinity()) {
            factible = validar_solucion(mejor_asignacion, p_count, costo_recalculado);
        }
        cout << "  [CHECK] Factible: " << (factible ? "SI" : "NO")
             << " | Costo reportado: " << mejor_costo
             << " | Costo recalculado: " << costo_recalculado << endl;
        if (tiempo_agotado) {
            cout << "  [TIMEOUT] Se alcanzo el limite de " << TIEMPO_LIMITE_SEG
                 << " segundos para este test." << endl;
        }

        file_res << p_count << "," << mejor_costo << "," << nodos_explorados << ","
                 << podas_mfc << "," << podas_costo << "," << chequeos_futuros << ","
                 << opciones_probadas << endl;

        ofstream file_conv("convergencia_mfc_p" + to_string(p_count) + ".csv");
        file_conv << "Tiempo,Costo" << endl;
        for (size_t i = 0; i < registro_tiempos.size(); ++i) {
            file_conv << registro_tiempos[i] << "," << registro_costos[i] << endl;
        }

        ofstream file_tree("perfil_mfc_p" + to_string(p_count) + ".csv");
        file_tree << "Nivel,Nodos" << endl;
        for (int i = 0; i <= total_aviones; ++i) {
            file_tree << i << "," << nodos_por_nivel[i] << endl;
        }
    }

    return 0;
}