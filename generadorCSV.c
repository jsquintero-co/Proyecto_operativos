#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const char *codigosPais[3] = {"ES", "FR", "IT"};
const char *nombresSector[5] = {"Industria", "Servicios", "Consumidores", "Comercio", "Construccion"};

const int maximos[3][5] = {
    {1760, 1430, 2020, 1080,   290},    // ES
    {3850, 4240, 1670, 2860, 24020},    // FR
    {5440, 3100, 2000, 1560,  1000}     // IT
};

int main(int argc, char *argv[]){
    char pais[3] = "";
    int porcentaje = -1;

    // Recorrido seguro de argumentos con validación de límites
    for(int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc){
            strcpy(pais, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc){
            porcentaje = atoi(argv[i + 1]);
            i++;
        }
    }

    if (strlen(pais) == 0 || porcentaje < 0 || porcentaje > 100){
        printf("Uso: ./generadorCSV -p <pais> -e <porcentaje>\n");
        return 1;
    }

    printf("Pais: %s\n", pais);
    printf("Porcentaje: %d%%\n", porcentaje);

    int idx = -1;
    for (int p = 0; p < 3; p++){
        if (strcmp(pais, codigosPais[p]) == 0){
            idx = p;
        }
    }

    if (idx == -1){
        printf("País no válido, debe ser ES, FR o IT\n");
        return 1;
    }

    char nombreArchivo[30];
    sprintf(nombreArchivo, "encuestas_%s.csv", pais);

    FILE *archivo = fopen(nombreArchivo, "w");
    if(archivo == NULL){
        perror("Error al abrir el archivo");
        return 1;
    }

    fprintf(archivo, "ID_Encuesta,ID_Pais,Pais,Sector,ID_Sector,Sentimiento\n");

    srand(time(NULL));
    int idEncuesta = 1;

    for (int s = 0; s < 5; s++){
        int cantidad = (maximos[idx][s] * porcentaje + 50) / 100;
        printf("  %-13s: %d/%d encuestas\n", nombresSector[s], cantidad, maximos[idx][s]);

        for (int j = 0; j < cantidad; j++){
            float sentimiento = (float)rand() / RAND_MAX * 100.0;
            fprintf(archivo, "%d,%d,%s,%s,%d,%.2f\n", idEncuesta, idx + 1, pais, nombresSector[s], s, sentimiento);
            idEncuesta++;
        }
    }

    printf("Total encuestas: %d\n", idEncuesta - 1);
    fclose(archivo);
    printf("Archivo %s generado correctamente.\n", nombreArchivo);

    return 0;
}