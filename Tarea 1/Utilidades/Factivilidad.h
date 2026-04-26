#ifndef FACTIBILIDAD_H
#define FACTIBILIDAD_H

#include <vector>
#include <utility>
#include "../Clase/Avion.h"

bool es_factible(const Avion& avion_k, int t_k,
                 const std::vector<std::pair<Avion, int>>& aviones_en_pista);

#endif