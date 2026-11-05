#ifndef COMUN_H
#define COMUN_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Pipe nominal
#define PIPE_NAME "PipeN"

// Límites del enunciado
#define MAX_AGENTES  4
#define TIEMPO_MIN   30
#define TIEMPO_MAX   120

// Países y sectores
#define NUM_PAISES   3
#define NUM_SECTORES 5

// Orden de los sectores
enum { INDUSTRIA, SERVICIOS, CONSUMIDORES, COMERCIO, CONSTRUCCION };

// Pesos del ISE por sector
static const float PESOS[NUM_SECTORES] = {
    0.40f, 0.30f, 0.20f, 0.05f, 0.05f
};

// Tipos de mensajes por el pipe
typedef enum {
    MP_CONEXION = 1,
    MP_ENCUESTA = 2,
    MP_FIN = 3
} TipoMensaje;

// Estructura del mensaje
typedef struct {
    int tipo;
    char pais[3];
    int id_sector;
    float sentimiento;
} Mensaje;

#endif
