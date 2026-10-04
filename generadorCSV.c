#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
	fprintf(archivo, "ID_Encuesta,ID_pais,Pais,Sector,ID_Sector,Sentimiento\n");

	//Se generan 10 encuestas random temporales para probar la creacion del CSV
	for (int i = 1; i <= 10; i++){
		float sentimiento;

		// Se genera un valor random entre 0 y 100
		sentimiento = (float)rand() / RAND_MAX * 100.0;

		//Escribe la encuesta en el archivo CSV
		fprintf(archivo, "%d,1,%s,Industria,0,%.2f\n", i, pais, sentimiento);
	}

	//Cierra el archivo
	fclose(archivo);

	//Mensaje que indica que está todo "OK" ó que se generó el archivo correctamente
	printf("Archivo %s generado correctamente.\n", nombreArchivo);

	return 0;
}


