#include "COMUN.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

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

    // Leemos los argumentos -n y -p
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            archivo = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            nombrePipe = argv[i + 1];
            i++;
        }
        else {
            printf("Argumentos incorrectos\n");
            return 1;
        }
    }

    // Revisamos que los dos argumentos existan
    if (archivo == NULL || nombrePipe == NULL) {
        printf("Uso: ./agentePais -n archivo.csv -p PipeN\n");
        return 1;
    }

    // Revisamos que el nombre del pipe sea correcto
    if (strcmp(nombrePipe, PIPE_NAME) != 0) {
        printf("Nombre del pipe incorrecto\n");
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
    fgets(linea, sizeof(linea), fp);

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

        // Extraemos el pais, el sector y el sentimiento
        if (sscanf(linea, "%*[^,],%2[^,],%*[^,],%*[^,],%d,%f",
                   pais,
                   &encuestas[leidas].id_sector,
                   &encuestas[leidas].sentimiento) != 3) {

            printf("Error leyendo una encuesta\n");
            free(encuestas);
            fclose(fp);
            return 1;
        }

        // Indicamos que es un mensaje de encuesta
        encuestas[leidas].tipo = MP_ENCUESTA;

        // Guardamos el pais
        strcpy(encuestas[leidas].pais, pais);

        leidas++;
    }

    // Cerramos el archivo porque ya terminamos de leerlo
    fclose(fp);

    // Ignoramos SIGPIPE para poder controlar errores de escritura
    signal(SIGPIPE, SIG_IGN);

    // Abrimos el pipe para escribir
    // O_NONBLOCK permite detectar si no hay un servidor conectado
    int dp = open(nombrePipe, O_WRONLY | O_NONBLOCK);

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
    mensaje.sentimiento = 0;

    if (enviarMensaje(dp, mensaje) == -1) {
        close(dp);
        free(encuestas);
        return 1;
    }

    // Enviamos las encuestas una por una
    for (int i = 0; i < total; i++) {

        if (enviarMensaje(dp, encuestas[i]) == -1) {
            close(dp);
            free(encuestas);
            return 1;
        }

        // Mostramos el progreso cada 100 encuestas
        if ((i + 1) % 100 == 0 || i + 1 == total) {
            printf("Encuestas enviadas: %d de %d\n", i + 1, total);
        }
    }

    // Avisamos que terminamos de enviar las encuestas
    mensaje.tipo = MP_FIN;

    if (enviarMensaje(dp, mensaje) == -1) {
        close(dp);
        free(encuestas);
        return 1;
    }

    printf("Envio terminado correctamente\n");

    // Liberamos la memoria y cerramos el pipe
    free(encuestas);
    close(dp);

    return 0;
}
