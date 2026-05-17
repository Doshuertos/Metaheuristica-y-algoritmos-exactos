[README (1).md](https://github.com/user-attachments/files/27892159/README.1.md)
# Algoritmos de Búsqueda con Restricciones — CSP

Este repositorio contiene la implementación de tres algoritmos de búsqueda con restricciones desarrollados en C++17. Cada algoritmo se encuentra en una rama independiente:

| Rama | Algoritmo |
|------|-----------|
| `mfc` | Minimal Forward Checking (MFC) |
| `Forward_Checking` | Forward Checking (FC) |
| `Resultados_BT` | Backtracking Cronológico (BT) |

Para cambiar de rama usa:

```bash
git checkout <nombre-de-rama>
```

---

## Rama `mfc` — Minimal Forward Checking

Implementación del algoritmo Minimal Forward Checking.

### Requisitos

- **g++** — compilador de C++
- **Windows PowerShell** o terminal compatible

Verificar instalación:

```bash
g++ --version
```

### Pasos para ejecutar

**1. Entrar a la carpeta del proyecto**

```bash
cd "Tarea 1"
```

**2. Compilar**

```bash
g++ -std=c++17 -I"Utilidades" -I"Clase" "Algoritmos_de_Busqueda/Minimal_Forward_Checking.cpp" "Clase/Clase_Avion.cpp" "Utilidades/Cargar_archivo.cpp" -o "mfc.exe"
```

**3. Ejecutar**

```bash
.\mfc.exe "Test_Case/case3.txt"
```

### Resultado esperado

El programa ejecuta el algoritmo MFC con el archivo de prueba indicado y muestra los resultados directamente en consola.

---

## Rama `Forward_Checking` — Forward Checking

Implementación del algoritmo Forward Checking.

### Compilar

```bash
g++ -std=c++17 -IAlgoritmos_de_Busqueda -IClase -IMain -IUtilidades \
Main/main.cpp \
Algoritmos_de_Busqueda/FC.cpp \
Clase/Clase_Avion.cpp \
Utilidades/Cargar_archivo.cpp \
Utilidades/Factivilidad.cpp \
-o Forward_Checking
```

### Ejecutar

```bash
./Forward_Checking
```

### Comportamiento

El programa tiene cargados los datos del **Case 4** e itera automáticamente sobre 3 escenarios de pistas:

- 1 pista
- 2 pistas
- 3 pistas

Esto se controla en `main.cpp` mediante el siguiente bucle:

```cpp
for (int p_count = 1; p_count <= 3; ++p_count)
```

Para ejecutar solo un caso específico, modificar el valor inicial de `p_count`:

- `1` → una pista
- `2` → dos pistas
- `3` → tres pistas

### Resultados

El programa genera automáticamente un archivo CSV en la carpeta:

```
Resultados de Ejecucion/
```

El nombre del archivo sigue el formato:

```
Datos33_pistas_N.csv
```

donde `N` es el número de pistas utilizadas.

---

## Rama `Resultados_BT` — Backtracking Cronológico

Implementación del algoritmo Backtracking Cronológico.

### Compilar

```bash
g++ -I Clase -I Utilidades -I Algoritmos_de_Busqueda \
Main/main.cpp \
Clase/Clase_Avion.cpp \
Utilidades/Cargar_archivo.cpp \
Utilidades/Factivilidad.cpp \
Algoritmos_de_Busqueda/FC.cpp \
Algoritmos_de_Busqueda/Backtracking_Cronológico.cpp \
-o main.exe
```

### Ejecutar

```bash
./main.exe
```
