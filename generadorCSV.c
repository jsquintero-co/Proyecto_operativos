#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//Siglas de los países, su posición + 1 es el ID_Pais
const char *codigosPais[3] = {"ES", "FR", "IT"};

//Nombres de los sectores, su posición es el ID_Sector
const char *nombresSector[5] = {"Industria", "Servicios", "Consumidores", "Comercio", "Construccion"};

//Tabla 1: máximo de encuestas por país (fila) y por sector (columna)
const int maximos[3][5] = {
	{1760, 1430, 2020, 1080,   290},	// ES
	{3850, 4240, 1670, 2860, 24020},	// FR
	{5440, 3100, 2000, 1560,  1000}		// IT
};

int main(int argc, char *argv[]){
	// Almacena el código del país, este mismo recibido cómo argumento a la hora de ejecutar
	char pais[3];

	//Almacena el porcentaje de encuestas que se desea generar
	int porcentaje = 0;

	//Recorre los argumentos recibidos a la hora de ejecutar 
	for(int i = 0; i < argc; i++){

		//Busca la flag -p para obtener el país
		if (strcmp(argv[i], "-p") == 0){
			strcpy(pais, argv[i + 1]);
		}

		//Busca el flag -e para obtener el porcentaje
		if (strcmp(argv[i], "-e") == 0){
			porcentaje = atoi(argv[i+1]);
		}
	}

	//Muestra los parámetros recibidos
	printf("Pais: %s\n", pais);
	printf("Porcentaje: %d%%\n", porcentaje);

	//Busca la fila del país en la tabla
	int idx = -1;
	for (int p = 0; p < 3; p++){
		if (strcmp(pais, codigosPais[p]) == 0){
			idx = p;
		}
	}

	if (idx == -1){
		printf("Pais no valido, debe ser ES, FR o IT\n");
		return 1;
	}

	//nombre del archivo CSV
	char nombreArchivo[30];

	sprintf(nombreArchivo, "encuestas_%s.csv", pais);

	//Se crea el archivo, si no existe, se crea, de lo contrario se sobrescribe
	FILE *archivo = fopen(nombreArchivo, "w");

	//Verifica que el archivo se pueda abrir
	if(archivo == NULL){
		perror("Error al abrir el archivo ");
		return 1;
	}

	//Escribe el encabezado del CSV
	fprintf(archivo, "ID_Encuesta,ID_Pais,Pais,Sector,ID_Sector,Sentimiento\n");

	//Semilla para que cada ejecución genere valores distintos
	srand(time(NULL));

	//El ID de la encuesta sigue corriendo entre sectores
	int idEncuesta = 1;

	//Recorre los 5 sectores y genera el porcentaje de encuestas de cada uno
	for (int s = 0; s < 5; s++){
		//Se suma 50 antes de dividir para redondear (ej: 2887.5 -> 2888)
		int cantidad = (maximos[idx][s] * porcentaje + 50) / 100;

		printf("  %-13s: %d/%d encuestas\n", nombresSector[s], cantidad, maximos[idx][s]);

		for (int j = 0; j < cantidad; j++){
			float sentimiento;

			// Se genera un valor random entre 0 y 100
			sentimiento = (float)rand() / RAND_MAX * 100.0;

			//Escribe la encuesta en el archivo CSV
			fprintf(archivo, "%d,%d,%s,%s,%d,%.2f\n", idEncuesta, idx + 1, pais, nombresSector[s], s, sentimiento);
			idEncuesta++;
		}
	}

	printf("Total encuestas: %d\n", idEncuesta - 1);

	//Cierra el archivo
	fclose(archivo);

	//Mensaje que indica que está todo "OK" ó que se generó el archivo correctamente
	printf("Archivo %s generado correctamente.\n", nombreArchivo);

	return 0;
}


