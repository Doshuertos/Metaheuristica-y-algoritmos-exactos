#ifndef BACKTRACKING_CRONOLOGICO_H
#define BACKTRACKING_CRONOLOGICO_H

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "../Clase/Avion.h"

struct Resultado {
    double costo = std::numeric_limits<double>::infinity();
    std::vector<std::vector<std::pair<Avion, int>>> plan;
};

struct Metricas {
    long long nodos_explorados     = 0;
    long long soluciones_factibles = 0;
    long long factible_ok          = 0;
    long long factible_fallo       = 0;
};

std::pair<Resultado, Metricas>
backtracking_cronologico(const std::vector<Avion>& aviones_entrada,
                         int tiempo_limite_seg = 600,
                         const std::string& nombre_caso = "caso",
                         int num_pistas = 3);

#endif
