#include "COMUN.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

// Estructura para llevar estadísticas por país y sector
typedef struct {
    int total_encuestas;
    float suma_sentimientos;
} EstadisticaSector;

typedef struct {
    char pais[3];
    int activo;
    EstadisticaSector sectores[NUM_SECTORES];
} EstadisticaPais;

// Variables globales para el estado y sincronización
EstadisticaPais estadisticas[NUM_PAISES];
pthread_mutex_t mutex_datos = PTHREAD_MUTEX_INITIALIZER;
int tiempo_limite = 30;
int servidor_activo = 1;

// Hilo encargado de controlar el tiempo de ejecución del servidor
void *hilo_temporizador(void *arg) {
    int segundos = *((int *)arg);
    sleep(segundos);
    
    pthread_mutex_lock(&mutex_datos);
    servidor_activo = 0;
    pthread_mutex_unlock(&mutex_datos);
    
    printf("\n[SERVIDOR] ¡Tiempo cumplido (%d segundos)! Finalizando recepción...\n", segundos);
    return NULL;
}

// Función para procesar y acumular los datos de las encuestas recibidas
void registrar_encuesta(const Mensaje *msg) {
    pthread_mutex_lock(&mutex_datos);
    
    int idx_pais = -1;
    if (strcmp(msg->pais, "ES") == 0) idx_pais = 0;
    else if (strcmp(msg->pais, "FR") == 0) idx_pais = 1;
    else if (strcmp(msg->pais, "IT") == 0) idx_pais = 2;
    
    if (idx_pais != -1 && msg->id_sector >= 0 && msg->id_sector < NUM_SECTORES) {
        strcpy(estadisticas[idx_pais].pais, msg->pais);
        estadisticas[idx_pais].activo = 1;
        estadisticas[idx_pais].sectores[msg->id_sector].total_encuestas++;
        estadisticas[idx_pais].sectores[msg->id_sector].suma_sentimientos += msg->sentimiento;
    }
    
    pthread_mutex_unlock(&mutex_datos);
}

// Generar reporte general del ISE al finalizar el tiempo
void generar_reporte_general() {
    printf("\n========================================================\n");
    printf("         REPORTE GENERAL - INDICADOR DE SENTIMIENTO (ISE) \n");
    printf("========================================================\n");

    const char *nombres_sectores[NUM_SECTORES] = {"Industria", "Servicios", "Consumidores", "Comercio", "Construccion"};

    for (int p = 0; p < NUM_PAISES; p++) {
        if (!estadisticas[p].activo) continue;

        printf("\n--- País: %s ---\n", estadisticas[p].pais);
        float ise_pais = 0.0f;
        int total_pais_encuestas = 0;

        for (int s = 0; s < NUM_SECTORES; s++) {
            int count = estadisticas[p].sectores[s].total_encuestas;
            total_pais_encuestas += count;
            float promedio_sector = 0.0f;

            if (count > 0) {
                promedio_sector = estadisticas[p].sectores[s].suma_sentimientos / count;
            }

            // Cálculo ponderado por sector
            ise_pais += promedio_sector * PESOS[s];
            printf("  * %-13s: Promedio = %.2f%% (Encuestas: %d, Peso: %.0f%%)\n", 
                   nombres_sectores[s], promedio_sector, count, PESOS[s] * 100);
        }

        printf("--> ISE Global para %s: %.2f%%\n", estadisticas[p].pais, ise_pais);

        // Recomendación final según la rúbrica
        printf("--> Recomendación: ");
        if (ise_pais > 50.0f) {
            printf("VENDER (Sentimiento positivo > 50%%)\n");
        } else if (ise_pais < 50.0f) {
            printf("COMPRAR (Sentimiento negativo < 50%%)\n");
        } else {
            printf("NEUTRO (Sentimiento = 50%%)\n");
        }
        printf("    Total encuestas procesadas: %d\n", total_pais_encuestas);
    }
    printf("\n========================================================\n");
}

int main(int argc, char *argv[]) {
    int tamB = 10;
    char *nombrePipe = PIPE_NAME;
    int tiempo = 30;
    int hilos_max = 2;

    // Lectura robusta de argumentos en cualquier orden
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            tamB = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            nombrePipe = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            tiempo = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            hilos_max = atoi(argv[i + 1]);
            i++;
        }
    }

    if (tiempo < TIEMPO_MIN || tiempo > TIEMPO_MAX) {
        printf("Error: El tiempo debe estar entre %d y %d segundos.\n", TIEMPO_MIN, TIEMPO_MAX);
        return 1;
    }

    // Inicializar estructuras
    for (int i = 0; i < NUM_PAISES; i++) {
        estadisticas[i].activo = 0;
        for (int s = 0; s < NUM_SECTORES; s++) {
            estadisticas[i].sectores[s].total_encuestas = 0;
            estadisticas[i].sectores[s].suma_sentimientos = 0.0f;
        }
    }

    // Crear Pipe Nominal (FIFO)
    unlink(nombrePipe);
    if (mkfifo(nombrePipe, 0666) == -1) {
        perror("Error al crear el pipe nominal");
        return 1;
    }

    printf("[SERVIDOR] Iniciando Control Proceso Principal ISE...\n");
    printf("[SERVIDOR] Configuración -> Buffer: %d | Pipe: %s | Tiempo: %s s | Hilos: %d\n", 
           tamB, nombrePipe, argv ? ("variable") : "30", hilos_max);

    // Crear hilo temporizador
    pthread_t thread_temp;
    tiempo_limite = tiempo;
    if (pthread_create(&thread_temp, NULL, hilo_temporizador, &tiempo_limite) != 0) {
        perror("Error al crear el hilo temporizador");
        unlink(nombrePipe);
        return 1;
    }

    // Abrir el pipe nominal para lectura (bloqueante hasta que un agente abra para escritura)
    int fd = open(nombrePipe, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el pipe para lectura");
        unlink(nombrePipe);
        return 1;
    }

    printf("[SERVIDOR] Esperando conexiones y lecturas de agentes...\n");

    // Bucle principal de lectura del pipe
    Mensaje mensaje;
    while (1) {
        pthread_mutex_lock(&mutex_datos);
        int activo = servidor_activo;
        pthread_mutex_unlock(&mutex_datos);

        if (!activo) break;

        ssize_t bytesLeidos = read(fd, &mensaje, sizeof(Mensaje));
        if (bytesLeidos > 0) {
            if (bytesLeidos == sizeof(Mensaje)) {
                if (mensaje.tipo == MP_CONEXION) {
                    printf("[SERVIDOR] Agente conectado para el país: %s\n", mensaje.pais);
                } else if (mensaje.tipo == MP_ENCUESTA) {
                    registrar_encuesta(&mensaje);
                } else if (mensaje.tipo == MP_FIN) {
                    printf("[SERVIDOR] Fin de transmisión recibido para un agente.\n");
                }
            }
        } else if (bytesLeidos == 0) {
            // Si el pipe se cierra por parte de los escritores, se puede reabrir o continuar
            usleep(100000);
        }
    }

    // Cierre y limpieza
    close(fd);
    unlink(nombrePipe);

    // Esperar al hilo temporizador
    pthread_join(thread_temp, NULL);

    // Generar reporte final
    generar_reporte_general();

    printf("[SERVIDOR] Proceso finalizado con éxito.\n");
    return 0;
}