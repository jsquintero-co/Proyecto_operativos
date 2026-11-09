#include "COMUN.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

// Funcion para enviar un mensaje por el pipe
int enviarMensaje(int dp, Mensaje mensaje) {
    ssize_t resultado = write(dp, &mensaje, sizeof(Mensaje));

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

    // Abrimos el archivo CSV
    FILE *fp = fopen(archivo, "r");
    if (fp == NULL) {
        perror("Error al abrir el archivo CSV");
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

    if (total == 0) {
        printf("El archivo no tiene encuestas válidas\n");
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

    // Volvemos al inicio del archivo y saltamos el encabezado
    rewind(fp);
    fgets(linea, sizeof(linea), fp);

    int leidas = 0;
    while (fgets(linea, sizeof(linea), fp) != NULL) {
        if (linea[0] == '\n' || linea[0] == '\r') {
            continue;
        }

        // Extraemos los datos de la encuesta
        if (sscanf(linea, "%*[^,],%2[^,],%*[^,],%*[^,],%d,%f",
                   pais,
                   &encuestas[leidas].id_sector,
                   &encuestas[leidas].sentimiento) != 3) {
            printf("Error leyendo una encuesta en el archivo\n");
            free(encuestas);
            fclose(fp);
            return 1;
        }

        encuestas[leidas].tipo = MP_ENCUESTA;
        strcpy(encuestas[leidas].pais, pais);
        leidas++;
    }

    fclose(fp);

    #ifdef SIGPIPE
        signal(SIGPIPE, SIG_IGN);
    #endif
    
    // Abrimos el pipe nominal para escritura
    int dp = open(nombrePipe, O_WRONLY);
    if (dp == -1) {
        perror("Error al abrir el pipe nominal");
        free(encuestas);
        return 1;
    }

    Mensaje mensaje;

    // 1. Enviamos mensaje de conexión inicial
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

    // 2. Enviamos las encuestas una por una
    for (int i = 0; i < total; i++) {
        if (enviarMensaje(dp, encuestas[i]) == -1) {
            close(dp);
            free(encuestas);
            return 1;
        }

        // Mostramos progreso cada 500 o al finalizar (o cada 100 según prefieras)
        if ((i + 1) % 500 == 0 || i + 1 == total) {
            printf("%d/%d\n", i + 1, total);
        }
    }

    // 3. Mensaje de fin
    mensaje.tipo = MP_FIN;
    if (enviarMensaje(dp, mensaje) == -1) {
        close(dp);
        free(encuestas);
        return 1;
    }

    printf("Enviadas %d/%d encuestas.\n", total, total);
    printf("Mensaje de finalización enviado.\n");
    printf("Envío %s finalizado satisfactoriamente!!\n", pais);

    // Liberación de recursos
    free(encuestas);
    close(dp);

    return 0;
}