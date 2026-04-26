#ifndef FACTIBILIDAD_H
#define FACTIBILIDAD_H

#include <vector>
#include <utility>
#include "../Clase/Avion.h"

bool es_factible(const Avion& avion_k, int t_k,
    const std::vector<std::pair<Avion, int>>& aviones_en_pista)
{
for (const auto& [avion_i, t_i] : aviones_en_pista) {
if (t_i < t_k) {
// i aterrizó antes que k
if (t_k < t_i + avion_i.Vector_Separacion_Tau[avion_k.id_avion - 1])
   return false;
} else {
// k aterrizará antes que i
if (t_i < t_k + avion_k.Vector_Separacion_Tau[avion_i.id_avion - 1])
   return false;
}
}
return true;
}

#endif