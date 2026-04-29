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

class Opcion {

    public : 
        int Pista_a_usar;
        int tiempo;
        float costo_individual;
    
    Opcion(int pista, int Tiempo, float costo) : Pista_a_usar(pista) , tiempo(Tiempo) , costo_individual(costo) {};

};



//Se debe filtrar el dominio mediante las restricciones, donde utilizaremos Tj​≥Ti​+τij​​ (si i llega antes) o Ti​≥Tj​+τji​ (si J llega antes) para podar deberia ser un bool y como es un arbol se trabaja con  recursividad
//Variables globales Gerenciamento de memória
int Cantidad_de_Aviones = 0;
vector<Avion>Aviones;
float mejor_costo = numeric_limits<double>::infinity() ;

bool Acotar_Arbol(int id, const Opcion& Asignada, const vector<vector<Opcion>>& dominios_viejos, vector<vector<Opcion>>& dominios_nuevos ) { 
    //const Opcion& , al trabajar con memoria usar const para que no tengamos problemas al acotar y ir pasando el Stack de los dominios
    //const vector<vector<Opcion>>& dominios_viejos, lo guardamos para el momento de retrocedes no perder toda la informacion se utiliza el const para que no se modifique nunca por accidente
    //vector<vector<Opcion>>& dominios_nuevos, para ir guardando los dominios que se van generando , para luego pasarlos a dominios viejos 

    dominios_nuevos = dominios_viejos; // para trabajar con los dominios anteriores
    Avion actual = Aviones[id];
    

    for(int j = id + 1; j < Cantidad_de_Aviones; j++){//iteramos los aviones que le siguen al que va a llegar 

        vector<Opcion>Nuevo_dom;
        Avion Avion_llegada = Aviones[j]; //Guardamos los siguientes

        for (int i = 0; i < dominios_viejos[j].size(); i++) { // iteramos opciones para elegir el siguiente avion 
            bool Viable = true;

            if(dominios_viejos[j][i].Pista_a_usar == Asignada.Pista_a_usar){ // Solo se activa si ambos aviones llegan a la misma pista 

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

void forward_checking(int id,float Costo_acumulado , const vector<vector<Opcion>>& dominios) {
    if(id == dominios.size()){
        if(Costo_acumulado < mejor_costo){
            mejor_costo = Costo_acumulado;
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

        if(Acotar_Arbol(id,dominios[id][i],dominios,dominios_futuros)){
            forward_checking(id+1,nuevo_costo,dominios_futuros);
        }
    }


}

int main() {
    string archivo = "Test_Case/case4.txt"; 
    cargarArchivo(archivo, Aviones, Cantidad_de_Aviones);

    if (Aviones.empty()) return 1;

    // Abrir archivo CSV y escribir cabecera

    sort(Aviones.begin(), Aviones.end(), [](const Avion& a, const Avion& b) {
        return a.Pk < b.Pk;
    });

    for (int p_count = 3; p_count <= 3; ++p_count) { 
        mejor_costo = numeric_limits<double>::infinity();


        vector<vector<Opcion>> dominios_iniciales(Cantidad_de_Aviones);
        for (int i = 0; i < Cantidad_de_Aviones; ++i) {
            for (int p = 0; p < p_count; ++p) {
                for (int t = Aviones[i].Ek; t <= Aviones[i].Lk; ++t) {
                    double c = Aviones[i].Calcular_Penalizaciones(t);
                    dominios_iniciales[i].push_back({p, t, c});
                }
            }
            sort(dominios_iniciales[i].begin(), dominios_iniciales[i].end(), [](const Opcion& a, const Opcion& b){
                return a.costo_individual < b.costo_individual;
            });
        }

        cout << "\n>>> Iniciando prueba: " << p_count << " pistas." << endl;
        
        forward_checking(0, 0.0, dominios_iniciales);

        cout << "Terminado " << p_count << " pistas. Mejor: " << mejor_costo << endl;
    }

    return 0;
}