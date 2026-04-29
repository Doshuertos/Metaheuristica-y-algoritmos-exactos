#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>
#include <fstream> // Necesario para guardar archivos

#include "Lector.h" `
#include "Avion.h"
#include "Opciones.h"

using namespace std;
//Codigo Original(el Codigo FC es para recolectar las estadisticas de la ejecucion), donde la IA aporto al codificar la solucion, siendo guia y correctora de este. (Se intento hacer gran parte a mano, y explicando detalle por detalle)

Opcion::Opcion(int pista, int Tiempo, float costo) : Pista_a_usar(pista) , tiempo(Tiempo) , costo_individual(costo) {}


//Se debe filtrar el dominio mediante las restricciones, donde utilizaremos Tj​≥Ti​+τij​​ (si i llega antes) o Ti​≥Tj​+τji​ (si J llega antes) para podar deberia ser un bool y como es un arbol se trabaja con  recursividad
//Variables globales Gerenciamento de memória
int Cantidad_de_Aviones = 0;
vector<Avion>Aviones;
float mejor_costo = 999999999999999999 ;

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

                    if(dominios_viejos[j][i].tiempo < Asignada.tiempo + actual.Vector_Separacion_Tau[Avion_llegada.id_avion-1]){ // Se comprueba Restriccion de Separacion
                        Viable = false;
                    }

                }else{

                    if(Asignada.tiempo < dominios_viejos[j][i].tiempo + Avion_llegada.Vector_Separacion_Tau[actual.id_avion-1]) { // Se comprueba Restriccion de Separacion
                        Viable = false;
                    }

                }

            }

            if(Viable){ // Guarda los resultados viables 
                Nuevo_dom.push_back(dominios_viejos[j][i]);
            }
        }
        if(Nuevo_dom.empty()){ // Si ningun dominio es viable
            return false;
        }
        dominios_nuevos[j] = Nuevo_dom;

    }
    return true;
}

void forward_checking(int id,float Costo_acumulado , const vector<vector<Opcion>>& dominios) {
    if(id == dominios.size()){ //Caso Base para terminar la recursividad
        if(Costo_acumulado < mejor_costo){
            mejor_costo = Costo_acumulado;
        }
        return;
    }

    if(Costo_acumulado >= mejor_costo){ //Tecnica de branch si no consigue resultado mejor la desecha y se ahorra tiempo y recursos
        return;
    }

    for(int i = 0; i < dominios[id].size(); i++){
        float nuevo_costo = Costo_acumulado + dominios[id][i].costo_individual;
        if (nuevo_costo >= mejor_costo){ //Solo si obtiene mayor costo empieza a descarta la opcion

            continue;

        } 
        vector<vector<Opcion>> dominios_futuros;

        if(Acotar_Arbol(id,dominios[id][i],dominios,dominios_futuros)){ //Se verifica si existen dominios futuros para continuar
            forward_checking(id+1,nuevo_costo,dominios_futuros); //Se aplica recursividad
        }
    }


}

