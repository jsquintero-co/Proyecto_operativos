#include "COMUN.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// Funcion para enviar un mensaje por el pipe
int enviarMensaje(int dp, Mensaje mensaje) {
    // Enviamos el mensaje
    ssize_t resultado = write(dp, &mensaje, sizeof(Mensaje));

    // Revisamos si hubo un error
    if (resultado != sizeof(Mensaje)) {
        perror("Error al enviar mensaje");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char *archivo = NULL;
    char *nombrePipe = NULL;

    // Leemos los argumentos -n y -p de forma robusta
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            archivo = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            nombrePipe = argv[i + 1];
            i++;
        }
    }

    // Revisamos que los dos argumentos existan
    if (archivo == NULL || nombrePipe == NULL) {
        printf("Uso: ./agentePais -n archivo.csv -p <pipeNom>\n");
        return 1;
    }

    // Abrimos el archivo CSV, fp es un puntero al archivo csv
    FILE *fp = fopen(archivo, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    char linea[256];
    char pais[3] = "";
    int total = 0;

    // Saltamos el encabezado del CSV
    if (fgets(linea, sizeof(linea), fp) == NULL) {
        printf("El archivo está vacío\n");
        fclose(fp);
        return 1;
    }

    // Contamos las encuestas
    while (fgets(linea, sizeof(linea), fp) != NULL) {
        if (linea[0] != '\n' && linea[0] != '\r') {
            total++;
        }
    }

    // Revisamos que existan encuestas
    if (total == 0) {
        printf("El archivo no tiene encuestas\n");
        fclose(fp);
        return 1;
    }

    // Reservamos memoria para guardar las encuestas
    Mensaje *encuestas = malloc(total * sizeof(Mensaje));
    if (encuestas == NULL) {
        perror("Error al reservar memoria");
        fclose(fp);
        return 1;
    }

    // Volvemos al inicio del archivo
    rewind(fp);

    // Saltamos nuevamente el encabezado
    fgets(linea, sizeof(linea), fp);

    int leidas = 0;

    // Leemos los datos de cada encuesta
    while (fgets(linea, sizeof(linea), fp) != NULL) {
        // Ignoramos las lineas vacias
        if (linea[0] == '\n' || linea[0] == '\r') {
            continue;
        }

        char pais_linea[3] = "";

        // Extraemos el pais, el sector y el sentimiento de forma robusta
        if (sscanf(linea, "%*[^,],%*[^,],%2[^,],%*[^,],%d,%f",
                   pais_linea,
                   &encuestas[leidas].id_sector,
                   &encuestas[leidas].sentimiento) != 3) {
            
            if (sscanf(linea, "%*[^,],%2[^,],%*[^,],%*[^,],%d,%f",
                       pais_linea,
                       &encuestas[leidas].id_sector,
                       &encuestas[leidas].sentimiento) != 3) {
                
                printf("Error leyendo una encuesta\n");
                free(encuestas);
                fclose(fp);
                return 1;
            }
        }

        // Indicamos que es un mensaje de encuesta
        encuestas[leidas].tipo = MP_ENCUESTA;

        // Guardamos el pais
        strcpy(encuestas[leidas].pais, pais_linea);

        if (leidas == 0) {
            strcpy(pais, pais_linea);
        }

        leidas++;
    }

    // Cerramos el archivo porque ya terminamos de leerlo
    fclose(fp);

    // Ignoramos SIGPIPE si está disponible (entornos POSIX/Linux) para controlar errores de escritura
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

    // Abrimos el pipe para escribir
    int dp = open(nombrePipe, O_WRONLY);
    if (dp == -1) {
        perror("Error al abrir el pipe");
        free(encuestas);
        return 1;
    }

    Mensaje mensaje;

    // Primero enviamos el mensaje de conexion
    mensaje.tipo = MP_CONEXION;
    strcpy(mensaje.pais, pais);
    mensaje.id_sector = 0;
    mensaje.sentimiento = 0.0f;

    if (enviarMensaje(dp, mensaje) == -1) {
        close(dp);
        free(encuestas);
        return 1;
    }

    printf("\nTHREAD - ENVÍO DE ENCUESTAS\n");
    printf("País: %s\n", pais);
    printf("Pipe: %s\n", nombrePipe);
    printf("Leídas %d encuestas.\n", total);
    printf("Conectando al proceso principal...\nConectado.\nEnviando...\n");

    // Enviamos las encuestas una por una
    for (int i = 0; i < total; i++) {
        if (enviarMensaje(dp, encuestas[i]) == -1) {
            close(dp);
            free(encuestas);
            return 1;
        }

        // Mostramos el progreso cada 500 encuestas o al finalizar
        if ((i + 1) % 500 == 0 || i + 1 == total) {
            printf("%d/%d\n", i + 1, total);
        }
    }

    // Avisamos que terminamos de enviar las encuestas
    mensaje.tipo = MP_FIN;
    strcpy(mensaje.pais, pais);

    if (enviarMensaje(dp, mensaje) == -1) {
        close(dp);
        free(encuestas);
        return 1;
    }

    printf("Enviadas %d/%d encuestas.\n", total, total);
    printf("Mensaje de finalización enviado.\n");
    printf("Envío %s finalizado satisfactoriamente!!\n", pais);

    // Liberamos la memoria y cerramos el pipe
    free(encuestas);
    close(dp);

    return 0;
}
