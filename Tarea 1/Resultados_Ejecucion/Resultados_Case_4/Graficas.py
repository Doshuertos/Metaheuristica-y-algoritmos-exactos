import pandas as pd
import matplotlib.pyplot as plt
import os

def analizar_algoritmo(archivo_csv, titulo_caso):
    # Obtener la ruta de la carpeta donde está este script
    directorio_actual = os.path.dirname(os.path.abspath(__file__))
    
    # Cargar los datos
    if not os.path.exists(archivo_csv):
        print(f"o se encuentra el archivo {archivo_csv}")
        return

    df = pd.read_csv(archivo_csv)
    
    # Crear la figura
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 15))
    fig.suptitle(f'Análisis de Desempeño: {titulo_caso}', fontsize=16)

    # 1. Curva de Convergencia (Costo vs Tiempo)
    ax1.plot(df['Tiempo_Seg'], df['Mejor_Costo'], marker='o', color='b', markersize=4)
    ax1.set_title('Optimización del Costo en el Tiempo')
    ax1.set_ylabel('Mejor Costo')
    ax1.grid(True)

    # 2. Exploración (Nodos vs Tiempo)
    ax2.plot(df['Tiempo_Seg'], df['Nodos_Explorados'], color='r')
    ax2.set_title('Nodos Explorados (Carga de Trabajo)')
    ax2.set_ylabel('Nodos')
    ax2.grid(True)

    # 3. Eficiencia (Nodos por Segundo)
    df['Nodos_por_Segundo'] = df['Nodos_Explorados'] / df['Tiempo_Seg']
    ax3.fill_between(df['Tiempo_Seg'], df['Nodos_por_Segundo'], color='g', alpha=0.3)
    ax3.set_title('Velocidad de Búsqueda (Nodos/Seg)')
    ax3.set_xlabel('Tiempo (segundos)')
    ax3.grid(True)

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])

    # GUARDAR EN LA CARPETA ACTUAL
    nombre_png = titulo_caso.replace(" ", "_") + ".png"
    ruta_salida = os.path.join(directorio_actual, nombre_png)
    
    plt.savefig(ruta_salida)
    print(f"Gráfico guardado en: {ruta_salida}")
    plt.close()

# Ejecución
analizar_algoritmo("datos4_pistas_3.csv", "Caso 4 - 3 Pista")