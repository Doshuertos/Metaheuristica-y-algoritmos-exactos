#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>
#include <fstream> 

#include "Lector.h"
#include "Avion.h"

using namespace std;

struct Opcion {
    int pista;
    int tiempo;
    double costo_individual;
};

// -------- MÉTRICAS GLOBALES --------
double mejor_costo = numeric_limits<double>::infinity();
long long nodos_explorados = 0;
long long podas_forward = 0;      // Por dominio vacío
long long podas_costo = 0;        // Por Branch & Bound
long long valores_eliminados = 0; // Total de (t,p) quitados por FC

// Vectores para gráficos
vector<double> registro_tiempos;
vector<double> registro_costos;
vector<long long> nodos_por_nivel;

int total_aviones;
vector<Avion> lista_aviones;
auto hora_inicio_algoritmo = chrono::high_resolution_clock::now();

bool filtrar_dominios(int idx_actual, const Opcion& asig, 
                      const vector<vector<Opcion>>& dominios_viejos, 
                      vector<vector<Opcion>>& dominios_nuevos) {
    
    dominios_nuevos = dominios_viejos;
    const Avion& av_i = lista_aviones[idx_actual];

    for (int j = idx_actual + 1; j < total_aviones; ++j) {
        vector<Opcion> aux;
        const Avion& av_j = lista_aviones[j];

        for (const auto& opt_j : dominios_viejos[j]) {
            if (opt_j.pista == asig.pista) {
                bool ok = true;
                if (asig.tiempo < opt_j.tiempo) {
                    if (opt_j.tiempo < asig.tiempo + av_i.Vector_Separacion_Tau[av_j.id_avion - 1]) ok = false;
                } else {
                    if (asig.tiempo < opt_j.tiempo + av_j.Vector_Separacion_Tau[av_i.id_avion - 1]) ok = false;
                }
                if (!ok) {
                    valores_eliminados++;
                    continue;
                }
            }
            aux.push_back(opt_j);
        }

        if (aux.empty()) {
            podas_forward++; 
            return false; 
        }
        dominios_nuevos[j] = std::move(aux);
    }
    return true;
}

void forward_checking(int idx, double costo_acumulado, const vector<vector<Opcion>>& dominios, int p_actual) {
    nodos_explorados++;
    nodos_por_nivel[idx]++;

    // Control de 5 minutos
    if (nodos_explorados % 10000 == 0) {
        auto ahora = chrono::high_resolution_clock::now();
        if (chrono::duration_cast<chrono::seconds>(ahora - hora_inicio_algoritmo).count() > 300) return;
    }

    // Caso Base: Solución Encontrada
    if (idx == total_aviones) {
        if (costo_acumulado < mejor_costo) {
            mejor_costo = costo_acumulado;
            auto ahora = chrono::high_resolution_clock::now();
            double t = chrono::duration_cast<chrono::milliseconds>(ahora - hora_inicio_algoritmo).count() / 1000.0;
            
            registro_tiempos.push_back(t);
            registro_costos.push_back(mejor_costo);
            
            cout << "  [LOG] Nuevo óptimo: " << mejor_costo << " en " << t << "s" << endl;
        }
        return;
    }

    // Poda por Costo
    if (costo_acumulado >= mejor_costo) {
        podas_costo++;
        return;
    }

    for(int i = 0; i < dominios[id].size(); i++){
        float nuevo_costo = Costo_acumulado + dominios[id][i].costo_individual;
        if (nuevo_costo >= mejor_costo){

            continue;
        }

        vector<vector<Opcion>> dominios_futuros;
        if (filtrar_dominios(idx, opt, dominios, dominios_futuros)) {
            forward_checking(idx + 1, nuevo_costo, dominios_futuros, p_actual);
        }
    }


}

int main() {
    string archivo = "Test_Case/case4.txt"; 
    cargarArchivo(archivo, lista_aviones, total_aviones);
    if (lista_aviones.empty()) return 1;

    sort(Aviones.begin(), Aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    ofstream file_res("metricas_finales.csv");
    file_res << "Pistas,Mejor_Costo,Nodos,Podas_FC,Podas_BB,Valores_Borrados" << endl;

    for (int p_count = 1; p_count <= 3; ++p_count) {
        mejor_costo = numeric_limits<double>::infinity();
        nodos_explorados = 0; podas_forward = 0; podas_costo = 0; valores_eliminados = 0;
        nodos_por_nivel.assign(total_aviones + 1, 0);
        registro_tiempos.clear(); registro_costos.clear();

        // Inicializar dominios
        vector<vector<Opcion>> dominios_ini(total_aviones);
        for (int i = 0; i < total_aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = Aviones[i].Ek; t <= Aviones[i].Lk; ++t) {
                    double c = Aviones[i].Calcular_Penalizaciones(t);
                    dominios_iniciales[i].push_back({p, t, c});
                }
            }
            sort(dominios_ini[i].begin(), dominios_ini[i].end(), [](const Opcion& a, const Opcion& b){
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> TEST: " << p_count << " PISTAS" << endl;
        hora_inicio_algoritmo = chrono::high_resolution_clock::now();
        forward_checking(0, 0.0, dominios_ini, p_count);

        // Guardar métricas resumen
        file_res << p_count << "," << mejor_costo << "," << nodos_explorados << "," 
                 << podas_forward << "," << podas_costo << "," << valores_eliminados << endl;

        // Guardar curva de convergencia para este p_count
        ofstream file_conv("convergencia_p" + to_string(p_count) + ".csv");
        file_conv << "Tiempo,Costo" << endl;
        for(size_t i=0; i<registro_tiempos.size(); ++i) 
            file_conv << registro_tiempos[i] << "," << registro_costos[i] << endl;
        
        // Guardar perfil del árbol
        ofstream file_tree("perfil_p" + to_string(p_count) + ".csv");
        file_tree << "Nivel,Nodos" << endl;
        for(int i=0; i<=total_aviones; ++i) 
            file_tree << i << "," << nodos_por_nivel[i] << endl;
    }

    return 0;
}