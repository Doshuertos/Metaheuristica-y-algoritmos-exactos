#ifndef AVION_H
#define AVION_H

#include <vector>

class Avion {
public:
    int id_avion;
    int Ek,Pk,Lk; // Parametros Ek = Tiempo de llegada Antes del Preferible, Pk = Tiempo de llegada Preferible, Tk = tiempo de llegada dsp del preferible
    float Penalizacion_Por_llegada_Temprana;
    float Penalizacion_Por_llegada_Tardia;
    std::vector<int>Vector_Separacion_Tau;


    Avion(int id, int e, int p, int l, float p_temprana, float p_tardia);

    float Calcular_Penalizaciones(int Tp);
};

#endif