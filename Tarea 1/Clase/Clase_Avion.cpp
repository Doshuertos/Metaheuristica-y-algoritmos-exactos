#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include "Avion.h"

using namespace std;


    Avion::Avion(int id, int e, int p, int l, float p_temprana, float p_tardia) : id_avion(id), Ek(e), Pk(p), Lk(l), Penalizacion_Por_llegada_Temprana(p_temprana), Penalizacion_Por_llegada_Tardia(p_tardia) {}

    float Avion::Calcular_Penalizaciones(int Tp){ //Tp = Tiempo de llegada
        if(Tp < this-> Pk ) {
            return Penalizacion_Por_llegada_Temprana * (this->Pk-Tp); // Por la FO (con el if se simula variable auxiliar binaria), Aqui es si llega antes  que el Preferible
        }
        else if (Tp > this-> Pk ) {
            return Penalizacion_Por_llegada_Tardia  * (Tp-this->Pk); //Aqui es si llega despues que el Preferible
        }
        return 0.0; //Si no llega ni una wea soajkfnhajks f
    }

