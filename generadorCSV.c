#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	char pais[3];
	int porcentaje = 0;

	for(int i = 0; i < argc; i++){
		if (strcmp(argv[i], "-p") == 0){
			strcpy(pais, argv[i + 1]);
		}

		if (strcmp(argv[i], "-e") == 0){
			porcentaje = atoi(argv[i+1]);
		}
	}

	printf("Pais: %s\n", pais);
	printf("Porcentaje: %d%%\n", porcentaje);

	return 0;
}


