#include "Lector.h"
#include <vector>

using namespace std;

extern vector<Avion> aviones;
extern int D;
extern int num_pistas;

vector<int> T_asig;
vector<int> pista_asig;

int main() {

    vector<Avion> lista;
    int n;

    cargarArchivo("case1.txt", lista, n);

    aviones = lista;
    D = n;
    num_pistas = 3;

    T_asig.assign(D, -1);
    pista_asig.assign(D, -1);

    vector<vector<Opcion>> dominios(D);

    for (int i = 0; i < D; i++) {
        for (int p = 0; p < num_pistas; p++) {
            for (int t = aviones[i].Ek; t <= aviones[i].Lk; t++) {
                dominios[i].push_back({p, t});
            }
        }
    }

    FC(0, dominios, 0.0);

    cout << "Mejor costo: " << mejor_costo << endl;
}