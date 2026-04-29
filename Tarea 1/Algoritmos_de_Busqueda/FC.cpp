#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem> // Para crear carpetas

#include "Lector.h" 
#include "Avion.h"
#include "Opciones.h"

namespace fs = std::filesystem;
using namespace std;
//Codigo modificado por IA para Obtener las metricas
// --- VARIABLES GLOBALES DE MÉTRICAS ---
long long nodos_explorados = 0;
long long podas = 0;
float mejor_costo = numeric_limits<double>::infinity();
auto hora_inicio_algoritmo = chrono::high_resolution_clock::now();

// Definición del constructor
Opcion::Opcion(int pista, int Tiempo, float costo) 
    : Pista_a_usar(pista), tiempo(Tiempo), costo_individual(costo) {}

int Cantidad_de_Aviones = 0;
vector<Avion> Aviones;

bool Acotar_Arbol(int id, const Opcion& Asignada, const vector<vector<Opcion>>& dominios_viejos, vector<vector<Opcion>>& dominios_nuevos) { 
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
                } else {
                    if(Asignada.tiempo < dominios_viejos[j][i].tiempo + Avion_llegada.Vector_Separacion_Tau[actual.id_avion-1]) {
                        Viable = false;
                    }
                }
            }
            if(Viable) Nuevo_dom.push_back(dominios_viejos[j][i]);
        }

        if(Nuevo_dom.empty()){
            podas++; // Incrementa poda cuando un dominio se vacía
            return false;
        }
        dominios_nuevos[j] = Nuevo_dom;
    }
    return true;
}

void forward_checking(int id, float Costo_acumulado, const vector<vector<Opcion>>& dominios, ofstream& archivo) {
    nodos_explorados++;

    if(id == dominios.size()){
        if(Costo_acumulado < mejor_costo){
            mejor_costo = Costo_acumulado;
            
            // Registro en el CSV: Tiempo, Costo, Nodos, Podas
            auto ahora = chrono::high_resolution_clock::now();
            double tiempo_total = chrono::duration<double>(ahora - hora_inicio_algoritmo).count();
            archivo << tiempo_total << "," << mejor_costo << "," << nodos_explorados << "," << podas << endl;
        }
        return;
    }

    if(Costo_acumulado >= mejor_costo){
        podas++; // Poda por Branch & Bound (costo)
        return;
    }

    for(int i = 0; i < dominios[id].size(); i++){
        float nuevo_costo = Costo_acumulado + dominios[id][i].costo_individual;
        if (nuevo_costo >= mejor_costo) continue;

        vector<vector<Opcion>> dominios_futuros;
        if(Acotar_Arbol(id, dominios[id][i], dominios, dominios_futuros)){
            forward_checking(id+1, nuevo_costo, dominios_futuros, archivo);
        }
    }
}
