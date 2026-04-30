#ifndef FORWARD_CHECKING_H
#define FORWARD_CHECKING_H

#include <vector>
#include "Avion.h"
#include <fstream> 
class Opcion {
public:
    int Pista_a_usar;
    int tiempo;
    float costo_individual;

    Opcion(int pista, int tiempo, float costo);
};

// funciones públicas del algoritmo
bool Acotar_Arbol(int id,
                   const Opcion& Asignada,
                   const std::vector<std::vector<Opcion>>& dominios_viejos,
                   std::vector<std::vector<Opcion>>& dominios_nuevos);

void forward_checking(int id,
                      float Costo_acumulado,
                      const std::vector<std::vector<Opcion>>& dominios, std::ofstream& file);

#endif