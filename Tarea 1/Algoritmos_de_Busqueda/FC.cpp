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

// --- MÉTRICAS AGREGADAS PARA VISUALIZACIÓN ---
long long nodos_explorados = 0;
auto hora_inicio_algoritmo = chrono::high_resolution_clock::now();
// --------------------------------------------

class Opcion {
    public : 
        int Pista_a_usar;
        int tiempo;
        float costo_individual;
    
    Opcion(int pista, int Tiempo, float costo) : Pista_a_usar(pista) , tiempo(Tiempo) , costo_individual(costo) {};
};

int Cantidad_de_Aviones = 0;
vector<Avion> Aviones;
float mejor_costo = numeric_limits<double>::infinity();

bool Acotar_Arbol(int id, const Opcion& Asignada, const vector<vector<Opcion>>& dominios_viejos, vector<vector<Opcion>>& dominios_nuevos ) { 
    dominios_nuevos = dominios_viejos; 
    Avion actual = Aviones[id];

    for(int j = id + 1; j < Cantidad_de_Aviones; j++){
        vector<Opcion> Nuevo_dom;
        Avion Avion_llegada = Aviones[j]; 

        for (int i = 0; i < dominios_viejos[j].size(); i++) { 
            bool Viable = true;
            if(dominios_viejos[j][i].Pista_a_usar == Asignada.Pista_a_usar){ 
                if(Asignada.tiempo < dominios_viejos[j][i].tiempo){
                    if(dominios_viejos[j][i].tiempo < Asignada.tiempo + actual.Vector_Separacion_Tau[Avion_llegada.id_avion-1]){
                        Viable = false;
                    }
                }else{
                    if(Asignada.tiempo < dominios_viejos[j][i].tiempo + Avion_llegada.Vector_Separacion_Tau[actual.id_avion-1]) {
                        Viable = false;
                    }
                }
            }

            if(Viable){
                Nuevo_dom.push_back(dominios_viejos[j][i]);
            }
        }
        if(Nuevo_dom.empty()){
            return false;
        }
        dominios_nuevos[j] = Nuevo_dom;
    }
    return true;
}

void forward_checking(int id, float Costo_acumulado , const vector<vector<Opcion>>& dominios) {
    // --- CONTADOR DE TRABAJO ---
    nodos_explorados++;
    
    // Muestra el progreso cada 50,000 nodos para no saturar la consola
    if (nodos_explorados % 50000 == 0) {
        auto ahora = chrono::high_resolution_clock::now();
        double t = chrono::duration_cast<chrono::seconds>(ahora - hora_inicio_algoritmo).count();
        cout << "[Progreso] Nodos: " << nodos_explorados << " | Tiempo: " << t << "s" << endl;
        
        // CORTE DE SEGURIDAD PARA TEST CASE 4 (5 minutos)
        if (t > 300) return;
    }

    if(id == dominios.size()){
        if(Costo_acumulado < mejor_costo){
            mejor_costo = Costo_acumulado;
            
            // --- CADA VEZ QUE ENCUENTRA UNA SOLUCIÓN MEJOR, LA LANZA ---
            auto ahora = chrono::high_resolution_clock::now();
            double t_ms = chrono::duration_cast<chrono::milliseconds>(ahora - hora_inicio_algoritmo).count() / 1000.0;
            cout << "  >>> [LOG] Nuevo costo hallado: " << fixed << setprecision(2) << mejor_costo 
                 << " | Tiempo: " << t_ms << "s" << " | Nodos: " << nodos_explorados << endl;
        }
        return;
    }

    if(Costo_acumulado >= mejor_costo){
        return;
    }

    for(int i = 0; i < dominios[id].size(); i++){
        float nuevo_costo = Costo_acumulado + dominios[id][i].costo_individual;
        if (nuevo_costo >= mejor_costo){
            continue;
        } 
        vector<vector<Opcion>> dominios_futuros;
        if(Acotar_Arbol(id, dominios[id][i], dominios, dominios_futuros)){
            forward_checking(id+1, nuevo_costo, dominios_futuros);
        }
    }
}

int main() {
    string archivo = "Test_Case/case4.txt"; 
    cargarArchivo(archivo, Aviones, Cantidad_de_Aviones);
    if (Aviones.empty()) return 1;

    sort(Aviones.begin(), Aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    // Cambia este número para probar 1, 2 o 3 pistas
    for (int p_count = 3; p_count <= 3; ++p_count) { 
        mejor_costo = numeric_limits<double>::infinity();
        nodos_explorados = 0;
        hora_inicio_algoritmo = chrono::high_resolution_clock::now();

        vector<vector<Opcion>> dominios_iniciales(Cantidad_de_Aviones);
        for (int i = 0; i < Cantidad_de_Aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = Aviones[i].Ek; t <= Aviones[i].Lk; ++t) {
                    double c = Aviones[i].Calcular_Penalizaciones(t);
                    dominios_iniciales[i].push_back(Opcion(p, t, c));
                }
            }
            sort(dominios_iniciales[i].begin(), dominios_iniciales[i].end(), [](const Opcion& a, const Opcion& b){
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> Iniciando prueba: " << p_count << " pistas." << endl;
        forward_checking(0, 0.0, dominios_iniciales);

        cout << "\n--- RESULTADO FINAL ---" << endl;
        cout << "Mejor Costo: " << mejor_costo << endl;
        cout << "Nodos Totales: " << nodos_explorados << endl;
    }

    return 0;
}