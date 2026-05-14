Para compilar el programa en Unix/Linux, ejecutar el siguiente comando:

g++ -std=c++17 -IAlgoritmos_de_Busqueda -IClase -IMain -IUtilidades \
Main/main.cpp \
Algoritmos_de_Busqueda/FC.cpp \
Clase/Clase_Avion.cpp \
Utilidades/Cargar_archivo.cpp \
Utilidades/Factivilidad.cpp \
-o Forward_Checking

Este comando generará el ejecutable Forward_Checking.

Ejecutar el programa

Una vez compilado, ejecutar:

./Forward_Checking

El programa actualmente:

Tiene cargados los datos del Case 4.
Itera automáticamente para 3 escenarios de pistas:
1 pista
2 pistas
3 pistas

Esto se controla en el main.cpp mediante el siguiente bucle:

for (int p_count = 1; p_count <= 3; ++p_count)

Si se desea ejecutar solo un caso específico, modificar el valor inicial de p_count por:

1 → una pista
2 → dos pistas
3 → tres pistas

Resultados

El programa genera automáticamente un archivo CSV en la carpeta:

Resultados de Ejecucion/

El nombre del archivo está hardcodeado con el formato:

Datos33_pistas_N.csv

Donde N corresponde al número de pistas utilizadas en la ejecución.
