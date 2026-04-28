#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#include "../Clase/Avion.h"
#include "../Utilidades/Lector.h"
#include "../Algoritmos_de_Busqueda/Backtracking_Cronológico.h"

using namespace std;

namespace {
constexpr int kTiempoLimiteSeg = 600;

/** Retorna (ruta archivo, nombre corto para ejecuciones/). */
pair<string, string> caso_a_rutas(int sel) {
    switch (sel) {
        case 0:
            return {"Tarea 1/Test_Case/case0_for_test", "case0"};
        case 1:
            return {"Tarea 1/Test_Case/case1.txt", "case1"};
        case 2:
            return {"Tarea 1/Test_Case/case2.txt", "case2"};
        case 3:
            return {"Tarea 1/Test_Case/case3.txt", "case3"};
        case 4:
            return {"Tarea 1/Test_Case/case4.txt", "case4"};
        default:
            return {"", ""};
    }
}
}

int main() {
    cout << "Seleccione el caso para Backtracking (tiempo maximo: 10 minutos):\n";
    cout << "  0 -> case0_for_test\n";
    cout << "  1 -> case1.txt\n";
    cout << "  2 -> case2.txt\n";
    cout << "  3 -> case3.txt\n";
    cout << "  4 -> case4.txt\n";
    cout << "Opcion (0-4): ";

    int sel = -1;
    while (!(cin >> sel) || sel < 0 || sel > 4) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Entrada invalida. Ingrese solo 0, 1, 2, 3 o 4: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    auto [ruta, nombre_caso] = caso_a_rutas(sel);

    vector<Avion> aviones;
    int n = 0;

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

    auto [resultado, metricas] =
        backtracking_cronologico(aviones, kTiempoLimiteSeg, nombre_caso, 3);

    cout << "\n==============================" << endl;
    cout << "Resultado BT (" << nombre_caso << ")" << endl;
    cout << "Mejor costo: " << resultado.costo << endl;
    cout << "Nodos explorados: " << metricas.nodos_explorados << endl;
    cout << "Soluciones factibles: " << metricas.soluciones_factibles << endl;
    cout << "==============================" << endl;

    return 0;
}