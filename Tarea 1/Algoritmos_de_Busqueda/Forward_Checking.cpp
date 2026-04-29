#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <fstream>

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
const int TIEMPO_LIMITE_SEG = 240;
vector<Opcion> asignaciones;

// Separacion global: aplica entre CUALQUIER par de aviones, sin importar pista
bool son_compatibles(const Avion& av_i, const Opcion& asig_i,
                     const Avion& av_j, const Opcion& asig_j) {
    if (asig_i.tiempo < asig_j.tiempo) {
        return asig_j.tiempo >= asig_i.tiempo + av_i.Vector_Separacion_Tau[av_j.id_avion - 1];
    } else {
        return asig_i.tiempo >= asig_j.tiempo + av_j.Vector_Separacion_Tau[av_i.id_avion - 1];
    }
}

bool consistente_con_pasado(int idx_actual, const Opcion& asig_actual) {
    const Avion& av_actual = lista_aviones[idx_actual];
    for (int i = 0; i < idx_actual; ++i) {
        if (!son_compatibles(lista_aviones[i], asignaciones[i], av_actual, asig_actual)) {
            return false;
        }
    }
    return true;
}

// MFC: para cada avion futuro j, verifica que exista AL MENOS UNA opcion
// compatible con todo el prefijo (0..idx_actual). Apenas la encuentra, avanza.
// Si ningun valor del dominio de j es compatible, poda inmediatamente.
bool verificar_futuro_minimo(int idx_actual, const vector<vector<Opcion>>& dominios) {
    for (int j = idx_actual + 1; j < total_aviones; ++j) {
        chequeos_futuros++;
        const Avion& av_j = lista_aviones[j];
        bool existe_opcion_valida = false;

        for (const auto& opt_j : dominios[j]) {
            opciones_probadas++;
            bool compatible = true;

            for (int i = 0; i <= idx_actual; ++i) {
                if (!son_compatibles(lista_aviones[i], asignaciones[i], av_j, opt_j)) {
                    compatible = false;
                    break;
                }
            }

            if (compatible) {
                existe_opcion_valida = true;
                break; // Minimal: apenas encuentra una opcion valida, para
            }
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
    nodos_explorados++;
    nodos_por_nivel[idx]++;

    if (nodos_explorados % 10000 == 0) {
        auto ahora = chrono::high_resolution_clock::now();
        if (chrono::duration_cast<chrono::seconds>(ahora - hora_inicio_algoritmo).count() > TIEMPO_LIMITE_SEG) {
            return;
        }
    }

    if (idx == total_aviones) {
        if (costo_acumulado < mejor_costo) {
            mejor_costo = costo_acumulado;
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
        double nuevo_costo = costo_acumulado + opt.costo_individual;
        if (nuevo_costo >= mejor_costo) {
            podas_costo++;
            continue;
        }

        if (!consistente_con_pasado(idx, opt)) {
            continue;
        }

        asignaciones[idx] = opt;

        if (verificar_futuro_minimo(idx, dominios)) {
            minimal_forward_checking(idx + 1, nuevo_costo, dominios, p_actual);
        }
    }
}

int main() {
    string archivo = "Test_Case/case2.txt";
    cargarArchivo(archivo, lista_aviones, total_aviones);
    if (lista_aviones.empty()) return 1;

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

        file_res << p_count << "," << mejor_costo << "," << nodos_explorados << ","
                 << podas_mfc << "," << podas_costo << "," << chequeos_futuros << ","
                 << opciones_probadas << endl;

        ofstream file_conv("convergencia_mfc_p" + to_string(p_count) + ".csv");
        file_conv << "Tiempo,Costo" << endl;
        for (size_t i = 0; i < registro_tiempos.size(); ++i)
            file_conv << registro_tiempos[i] << "," << registro_costos[i] << endl;

        ofstream file_tree("perfil_mfc_p" + to_string(p_count) + ".csv");
        file_tree << "Nivel,Nodos" << endl;
        for (int i = 0; i <= total_aviones; ++i)
            file_tree << i << "," << nodos_por_nivel[i] << endl;
    }

    return 0;
}