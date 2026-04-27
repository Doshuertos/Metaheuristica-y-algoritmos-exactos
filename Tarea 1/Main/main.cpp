#include <iostream>
#include <vector>
#include "../Clase/Avion.h"
#include "../Utilidades/Lector.h"
#include "../Algoritmos_de_Busqueda/Backtracking_Cronológico.h"

using namespace std;

int main() {
    vector<Avion> aviones;
    int n = 0;

    string ruta = "Tarea 1/Test_Case/case1.txt";

    cargarArchivo(ruta, aviones, n);
    if (aviones.empty()) {
        cout << "No se pudieron cargar aviones desde: " << ruta << endl;
        return 1;
    }

    cout << "==============================" << endl;
    cout << "Aviones cargados: " << n << endl;
    cout << "==============================" << endl;

    for (const auto& a : aviones) {
        cout << "ID: " << a.id_avion
             << " Ek: " << a.Ek
             << " Pk: " << a.Pk
             << " Lk: " << a.Lk
             << endl;
    }
    cout << "\nPrueba de separaciones:\n";
    int filas = min(3, n);
    for (int i = 0; i < filas; i++) {
        int columnas = min(3, n);
        for (int j = 0; j < columnas; j++) {
            cout << aviones[i].Vector_Separacion_Tau[j] << " ";
        }
        cout << endl;
    }

    auto [resultado, metricas] = backtracking_cronologico(aviones, 600, "case1", 3);

    cout << "\n==============================" << endl;
    cout << "Resultado BT (case1)" << endl;
    cout << "Mejor costo: " << resultado.costo << endl;
    cout << "Nodos explorados: " << metricas.nodos_explorados << endl;
    cout << "Soluciones factibles: " << metricas.soluciones_factibles << endl;
    cout << "==============================" << endl;

    return 0;
}