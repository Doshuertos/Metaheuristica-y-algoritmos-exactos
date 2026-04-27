#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>
#include <fstream> // Necesario para guardar archivos

#include "Lector.h" 
#include "Avion.h"

using namespace std;

struct Opcion {
    int pista;
    int tiempo;
    double costo_individual;
};

// Globales
double mejor_costo = numeric_limits<double>::infinity();
long long nodos_explorados = 0;
int total_aviones;
vector<Avion> lista_aviones;
auto hora_inicio_algoritmo = chrono::high_resolution_clock::now();
ofstream archivo_csv; // Flujo para el archivo de datos

bool filtrar_dominios(int idx_actual, const Opcion& asig, 
                      const vector<vector<Opcion>>& dominios_viejos, 
                      vector<vector<Opcion>>& dominios_nuevos) {
    
    dominios_nuevos = dominios_viejos;
    const Avion& av_i = lista_aviones[idx_actual];

    for (int j = idx_actual + 1; j < total_aviones; ++j) {
        vector<Opcion> aux;
        const Avion& av_j = lista_aviones[j];
        aux.reserve(dominios_viejos[j].size());

        for (const auto& opt_j : dominios_viejos[j]) {
            if (opt_j.pista == asig.pista) {
                bool ok = true;
                if (asig.tiempo < opt_j.tiempo) {
                    if (opt_j.tiempo < asig.tiempo + av_i.Vector_Separacion_Tau[av_j.id_avion - 1]) ok = false;
                } else {
                    if (asig.tiempo < opt_j.tiempo + av_j.Vector_Separacion_Tau[av_i.id_avion - 1]) ok = false;
                }
                if (!ok) continue;
            }
            aux.push_back(opt_j);
        }

        if (aux.empty()) return false; 
        dominios_nuevos[j] = std::move(aux);
    }
    return true;
}

void forward_checking(int idx, double costo_acumulado, const vector<vector<Opcion>>& dominios, int p_actual) {
    nodos_explorados++;

    if (nodos_explorados % 5000 == 0) { 
        auto ahora = chrono::high_resolution_clock::now();
        if (chrono::duration_cast<chrono::seconds>(ahora - hora_inicio_algoritmo).count() > 300) return; 
    }

    if (idx == total_aviones) {
        if (costo_acumulado < mejor_costo) {
            mejor_costo = costo_acumulado;
            
            // Calculamos el tiempo transcurrido
            auto ahora = chrono::high_resolution_clock::now();
            double tiempo_s = chrono::duration_cast<chrono::milliseconds>(ahora - hora_inicio_algoritmo).count() / 1000.0;

            // Guardamos en el CSV: Pistas, Tiempo, Costo, Nodos
            archivo_csv << p_actual << "," << tiempo_s << "," << mejor_costo << "," << nodos_explorados << endl;
            
            cout << "  [Log] Nuevo costo: " << mejor_costo << " | Tiempo: " << tiempo_s << "s" << endl;
        }
        return;
    }

    if (costo_acumulado >= mejor_costo) return;

    for (const auto& opt : dominios[idx]) {
        double nuevo_costo = costo_acumulado + opt.costo_individual;
        if (nuevo_costo >= mejor_costo) continue;

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

    // Abrir archivo CSV y escribir cabecera
    archivo_csv.open("resultados_fc.csv");
    archivo_csv << "Pistas,Tiempo_Segundos,Costo,Nodos_Explorados" << endl;

    sort(lista_aviones.begin(), lista_aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    for (int p_count = 3; p_count <= 3; ++p_count) { 
        mejor_costo = numeric_limits<double>::infinity();
        nodos_explorados = 0;

        vector<vector<Opcion>> dominios_iniciales(total_aviones);
        for (int i = 0; i < total_aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = lista_aviones[i].Ek; t <= lista_aviones[i].Lk; ++t) {
                    double c = lista_aviones[i].Calcular_Penalizaciones(t);
                    dominios_iniciales[i].push_back({p, t, c});
                }
            }
            sort(dominios_iniciales[i].begin(), dominios_iniciales[i].end(), [](const Opcion& a, const Opcion& b){
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> Iniciando prueba: " << p_count << " pistas." << endl;
        hora_inicio_algoritmo = chrono::high_resolution_clock::now();
        
        forward_checking(0, 0.0, dominios_iniciales, p_count);

        cout << "Terminado " << p_count << " pistas. Mejor: " << mejor_costo << endl;
    }

    archivo_csv.close();
    cout << "\nDatos exportados exitosamente a 'resultados_fc.csv'" << endl;
    return 0;
}