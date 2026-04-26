#ifndef LECTOR_H
#define LECTOR_H

#include <vector>
#include <string>
#include "../Clase/Avion.h"

void cargarArchivo(const std::string& ruta, std::vector<Avion>& lista, int& n);

#endif