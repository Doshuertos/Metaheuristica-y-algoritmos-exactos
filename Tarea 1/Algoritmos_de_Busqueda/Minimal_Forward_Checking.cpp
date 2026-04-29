#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <fstream>
#include <string>
#include "Lector.h"
#include "Avion.h"

using namespace std;

struct Opcion { int pista, tiempo; double costo_individual; };

// -------- CONFIGURACION Y METRICAS --------
const bool SEPARACION_GLOBAL = true;
const int  TIEMPO_LIMITE_SEG = 600;

double mejor_costo = numeric_limits<double>::infinity();
long long nodos_explorados = 0, soluciones_factibles = 0, podas_mfc = 0;
long long podas_costo = 0, chequeos_futuros = 0, opciones_probadas = 0;
bool tiempo_agotado = false;

vector<double> registro_tiempos, registro_costos, registro_nodos_en_mejora;
vector<long long> nodos_por_nivel;
vector<Opcion> asignaciones, mejor_asignacion;

int total_aviones;
vector<Avion> lista_aviones;
auto hora_inicio = chrono::high_resolution_clock::now();

// ──────────────────────────────────────────────
//  COMPATIBILIDAD
// ──────────────────────────────────────────────
bool opciones_compatibles(const Avion& av_i, const Opcion& asig, const Avion& av_j, const Opcion& opt_j) {
    if (!SEPARACION_GLOBAL && asig.pista != opt_j.pista) return true;
    int sep_ij = av_i.Vector_Separacion_Tau[av_j.id_avion - 1];
    int sep_ji = av_j.Vector_Separacion_Tau[av_i.id_avion - 1];
    return asig.tiempo < opt_j.tiempo ? opt_j.tiempo >= asig.tiempo + sep_ij 
                                      : asig.tiempo >= opt_j.tiempo + sep_ji;
}

bool consistente_con_pasado(int idx_actual, const Opcion& asig_actual) {
    for (int i = 0; i < idx_actual; ++i)
        if (!opciones_compatibles(lista_aviones[i], asignaciones[i], lista_aviones[idx_actual], asig_actual))
            return false;
    return true;
}

// ──────────────────────────────────────────────
//  VALIDADOR INDEPENDIENTE
// ──────────────────────────────────────────────
bool validar_solucion(const vector<Opcion>& sol, int p_count, double& costo_recalc) {
    costo_recalc = 0.0;
    for (int i = 0; i < total_aviones; ++i) {
        if (sol[i].pista < 0 || sol[i].pista >= p_count || 
            sol[i].tiempo < lista_aviones[i].Ek || sol[i].tiempo > lista_aviones[i].Lk) return false;
        costo_recalc += lista_aviones[i].Calcular_Penalizaciones(sol[i].tiempo);
    }
    for (int i = 0; i < total_aviones; ++i) {
        for (int j = i + 1; j < total_aviones; ++j) {
            if (!SEPARACION_GLOBAL && sol[i].pista != sol[j].pista) continue;
            if (sol[i].tiempo == sol[j].tiempo || !opciones_compatibles(lista_aviones[i], sol[i], lista_aviones[j], sol[j])) 
                return false;
        }
    }
    return true;
}

// ──────────────────────────────────────────────
//  MFC PURO
// ──────────────────────────────────────────────
bool verificar_futuro_minimo(int idx_actual, const vector<vector<Opcion>>& dominios) {
    for (int j = idx_actual + 1; j < total_aviones; ++j) {
        if (tiempo_agotado) return false;
        chequeos_futuros++;
        bool existe_valida = false;

        for (const auto& opt_j : dominios[j]) {
            if (tiempo_agotado) return false;
            opciones_probadas++;
            
            bool compatible = true;
            for (int i = 0; i <= idx_actual; ++i) {
                if (!opciones_compatibles(lista_aviones[i], asignaciones[i], lista_aviones[j], opt_j)) {
                    compatible = false; break;
                }
            }
            if (compatible) { existe_valida = true; break; } // Minimal FC
        }
        if (!existe_valida) { podas_mfc++; return false; }
    }
    return true;
}

// ──────────────────────────────────────────────
//  BUSQUEDA PRINCIPAL
// ──────────────────────────────────────────────
void minimal_forward_checking(int idx, double costo_acumulado, const vector<vector<Opcion>>& dominios) {
    if (tiempo_agotado) return;

    double seg = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - hora_inicio).count() / 1000.0;
    if (seg > TIEMPO_LIMITE_SEG) { tiempo_agotado = true; return; }

    nodos_explorados++;
    nodos_por_nivel[idx]++;

    if (idx == total_aviones) {
        soluciones_factibles++;
        if (costo_acumulado < mejor_costo) {
            mejor_costo = costo_acumulado;
            mejor_asignacion = asignaciones;
            registro_tiempos.push_back(seg);
            registro_costos.push_back(mejor_costo);
            registro_nodos_en_mejora.push_back(nodos_explorados);
            cout << "  [LOG MFC] Nuevo optimo: " << mejor_costo << " en " << seg << "s (nodos=" << nodos_explorados << ")\n";
        }
        return;
    }

    if (costo_acumulado >= mejor_costo) { podas_costo++; return; }

    for (const auto& opt : dominios[idx]) {
        if (tiempo_agotado) return;
        double nuevo_costo = costo_acumulado + opt.costo_individual;
        
        if (nuevo_costo >= mejor_costo) { podas_costo++; continue; }
        if (!consistente_con_pasado(idx, opt)) continue;

        asignaciones[idx] = opt;
        if (verificar_futuro_minimo(idx, dominios))
            minimal_forward_checking(idx + 1, nuevo_costo, dominios);
    }
}

// ──────────────────────────────────────────────
//  MAIN
// ──────────────────────────────────────────────
int main(int argc, char* argv[]) {
    string archivo = (argc >= 2) ? argv[1] : "Test_Case/case1.txt";
    cargarArchivo(archivo, lista_aviones, total_aviones);
    if (lista_aviones.empty()) return 1;
    
    cout << "[INFO] Caso: " << archivo << " | Aviones: " << total_aviones << "\n";
    sort(lista_aviones.begin(), lista_aviones.end(), [](const Avion& a, const Avion& b){ return a.Pk < b.Pk; });

    const int p_count = 3;
    nodos_por_nivel.assign(total_aviones + 1, 0);
    asignaciones.assign(total_aviones, {-1, -1, 0.0});
    mejor_asignacion.assign(total_aviones, {-1, -1, 0.0});

    // Construir dominios ordenados (Heurística de valor)
    vector<vector<Opcion>> dominios_ini(total_aviones);
    for (int i = 0; i < total_aviones; ++i) {
        for (int p = 0; p < p_count; ++p)
            for (int t = lista_aviones[i].Ek; t <= lista_aviones[i].Lk; ++t)
                dominios_ini[i].push_back({p, t, lista_aviones[i].Calcular_Penalizaciones(t)});
        sort(dominios_ini[i].begin(), dominios_ini[i].end(), [](const Opcion& a, const Opcion& b){ return a.costo_individual < b.costo_individual; });
    }

    cout << "\n>>> TEST MFC: " << p_count << " PISTAS\n";
    hora_inicio = chrono::high_resolution_clock::now();
    minimal_forward_checking(0, 0.0, dominios_ini);

    double costo_recalc = 0.0;
    bool factible = mejor_costo < numeric_limits<double>::infinity() && validar_solucion(mejor_asignacion, p_count, costo_recalc);

    cout << "  [CHECK] Factible: " << (factible ? "SI" : "NO") << " | Costo: " << mejor_costo << " | Recalculado: " << costo_recalc << "\n";
    if (tiempo_agotado) cout << "  [TIMEOUT] Limite de " << TIEMPO_LIMITE_SEG << "s alcanzado.\n";

    cout << "\nMETRICAS\nNodos: " << nodos_explorados << " | Soluciones: " << soluciones_factibles 
         << "\nPodas MFC: " << podas_mfc << " | Podas B&B: " << podas_costo 
         << "\nChequeos: " << chequeos_futuros << " | Opciones probadas: " << opciones_probadas << "\n";

    // ── Exportación de CSVs y TXT ──
    ofstream("metricas_mfc.csv") << "Pistas,Costo,Nodos,Soluciones,PodasMFC,PodasBB,Chequeos,Opciones\n"
                                 << p_count << "," << mejor_costo << "," << nodos_explorados << "," << soluciones_factibles << ","
                                 << podas_mfc << "," << podas_costo << "," << chequeos_futuros << "," << opciones_probadas << "\n";

    ofstream f_conv("convergencia_mfc_p3.csv"); f_conv << "Tiempo,Costo\n";
    for (size_t i = 0; i < registro_tiempos.size(); ++i) f_conv << registro_tiempos[i] << "," << registro_costos[i] << "\n";

    ofstream f_nodos("costo_vs_nodos_mfc_p3.csv"); f_nodos << "Nodos,Costo\n";
    for (size_t i = 0; i < registro_nodos_en_mejora.size(); ++i) f_nodos << registro_nodos_en_mejora[i] << "," << registro_costos[i] << "\n";

    ofstream f_perfil("perfil_mfc_p3.csv"); f_perfil << "Nivel,Nodos\n";
    for (int i = 0; i <= total_aviones; ++i) f_perfil << i << "," << nodos_por_nivel[i] << "\n";

    ofstream f_txt("metricas_mfc_p3.txt");
    f_txt << "MFC\nMejor Costo: " << mejor_costo << "\n\n";
    if (factible) {
        vector<vector<pair<int,int>>> plan(p_count);
        for (int i = 0; i < total_aviones; ++i) plan[mejor_asignacion[i].pista].push_back({lista_aviones[i].id_avion, mejor_asignacion[i].tiempo});
        for (int p = 0; p < p_count; ++p) {
            sort(plan[p].begin(), plan[p].end(), [](auto& a, auto& b){ return a.second < b.second; });
            f_txt << "Pista " << (p + 1) << ":\n";
            for (auto& [id, t] : plan[p]) f_txt << "  [T=" << t << "] Avion " << id << "\n";
        }
    }
    return 0;
}