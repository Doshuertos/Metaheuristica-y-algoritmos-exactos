#include "Lector.h"
#include <fstream>
#include <iostream>

using namespace std;

// FUNCIONES PARA CARGAR LA DATA PRECISA AAAAAAAAAAAAAA
   void cargarArchivo(const std::string& ruta, std::vector<Avion>& lista, int& n) { //Comentar informe que Gemini hizo la funcion de Cargar datos pq es el meo genio

    ifstream archivo(ruta);
    if (!archivo.is_open()) {
    cout << "No funciona, Pongase serio" << ruta << endl;
    return;
    }

    archivo >> n;
    for (int i = 0; i < n; ++i) {
        int e, p, l;
        float p_temp, p_tard;
        archivo >> e >> p >> l >> p_temp >> p_tard;

        Avion nuevo(i + 1, e, p, l, p_temp, p_tard);
        for (int j = 0; j < n; ++j) {
            int tau;
            archivo >> tau;
            nuevo.Vector_Separacion_Tau.push_back(tau);
        }
        lista.push_back(nuevo);
    }
    archivo.close();

}
