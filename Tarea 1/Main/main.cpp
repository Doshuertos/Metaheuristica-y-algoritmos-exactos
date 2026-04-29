#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>
#include <fstream> 
#include <filesystem> // <--- AGREGADO

#include "Lector.h" 
#include "Avion.h"
#include "Opciones.h"

using namespace std;
namespace fs = std::filesystem; // <--- AGREGADO: Alias para carpetas

// ---------------- EXTERN ----------------
// Asegúrate de que en el otro archivo .cpp estas variables NO tengan el 'extern'
extern vector<Avion> Aviones;
extern int Cantidad_de_Aviones;
extern float mejor_costo;
extern long long nodos_explorados;
extern long long podas;
extern chrono::high_resolution_clock::time_point hora_inicio_algoritmo;

// ---------------- PROTOTIPO ----------------
// Agregado para que el main reconozca la función
void forward_checking(int id, float Costo_acumulado, const vector<vector<Opcion>>& dominios, ofstream& archivo);

int main() {
    string archivo_datos = "Test_Case/case4.txt"; 
    cargarArchivo(archivo_datos, Aviones, Cantidad_de_Aviones);
    if (Aviones.empty()) return 1;

    // 1. Crear carpeta para los resultados
    string ruta_carpeta = "Resultados_Ejecucion";
    if (!fs::exists(ruta_carpeta)) {
        fs::create_directories(ruta_carpeta);
    }

    sort(Aviones.begin(), Aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    for (int p_count = 3; p_count <= 3; ++p_count) {
        // Reinicio de métricas
        mejor_costo = numeric_limits<float>::infinity(); // Usamos float para ser consistentes
        nodos_explorados = 0;
        podas = 0;
        hora_inicio_algoritmo = chrono::high_resolution_clock::now();

        string nombre_archivo = ruta_carpeta + "/datos4_pistas_" + to_string(p_count) + ".csv";
        ofstream archivo_csv(nombre_archivo);
        
        if (!archivo_csv.is_open()) {
            cerr << "Error al crear el archivo CSV para " << p_count << " pistas." << endl;
            continue;
        }

        archivo_csv << "Tiempo_Seg,Mejor_Costo,Nodos_Explorados,Podas" << endl;

        vector<vector<Opcion>> dominios_iniciales(Cantidad_de_Aviones);
        for (int i = 0; i < Cantidad_de_Aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = Aviones[i].Ek; t <= Aviones[i].Lk; ++t) {
                    dominios_iniciales[i].push_back(Opcion(p, t, Aviones[i].Calcular_Penalizaciones(t)));
                }
            }
            sort(dominios_iniciales[i].begin(), dominios_iniciales[i].end(), [](const Opcion& a, const Opcion& b){
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> Iniciando Case con " << p_count << " pistas..." << endl;
        
        // Llamada a la función
        forward_checking(0, 0.0, dominios_iniciales, archivo_csv);

        cout << "Finalizado " << p_count << " pistas. Mejor Costo: " << mejor_costo << endl;
        archivo_csv.close();
    }

    return 0;
}