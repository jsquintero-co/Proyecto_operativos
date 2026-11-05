# Proyecto: Indicador de Sentimiento Económico (ISE)

## 1. Descripción del proyecto

El proyecto consiste en desarrollar un simulador del Indicador de Sentimiento Económico (ISE) utilizando el lenguaje C y herramientas de programación POSIX.

El sistema permite generar encuestas para diferentes países, enviarlas mediante pipes nominales y procesarlas desde un servidor principal. Con la información recibida se calcula el Indicador de Sentimiento Económico y se genera una recomendación según los resultados.

## 2. Objetivos

- Implementar comunicación entre procesos mediante pipes nominales (FIFO).
- Utilizar procesos e hilos POSIX.
- Manejar la sincronización entre procesos e hilos.
- Generar y procesar archivos de encuestas en formato CSV.
- Calcular el Indicador de Sentimiento Económico.
- Mostrar reportes parciales y un reporte general.

## 3. Componentes del sistema

El proyecto está dividido en tres programas principales:

### Generador de encuestas (`generadorCSV`)

Se encarga de crear archivos CSV con información aleatoria de las encuestas para cada país.

Los países disponibles son:
- España (ES).
- Francia (FR).
- Italia (IT).

Recibe como parámetros el país y el porcentaje de encuestas que se desean generar.

### Agente de lectura (`agentePais`)

Lee el archivo CSV correspondiente a un país y envía las encuestas al servidor mediante el pipe nominal.

Sus funciones principales son:
- Leer el archivo de encuestas.
- Identificar la cantidad de registros.
- Conectarse al pipe nominal.
- Enviar las encuestas.
- Mostrar el progreso del envío.
- Informar cuando termina de enviar la información.

### Servidor principal (`servidorISE`)

Es el encargado de recibir y procesar la información enviada por los agentes.

Sus funciones principales son:
- Crear y administrar el pipe nominal.
- Recibir las encuestas de los diferentes agentes.
- Utilizar hilos para atender a los agentes.
- Procesar los datos recibidos.
- Generar reportes parciales.
- Calcular el resultado general del ISE al finalizar el tiempo establecido.

## 4. Indicador de Sentimiento Económico

El cálculo del ISE utiliza los siguientes pesos:

| Sector | Peso |
|---|---:|
| Industria | 40% |
| Servicios | 30% |
| Consumidores | 20% |
| Comercio | 5% |
| Construcción | 5% |

La recomendación se determina según el resultado:

- **Vender:** sentimiento positivo superior al 50%.
- **Comprar:** sentimiento negativo inferior al 50%.
- **Neutro:** sentimiento igual al 50%.

## 5. Comunicación entre procesos

La comunicación entre los agentes y el servidor se realiza mediante un pipe nominal (FIFO).

Se utiliza una estructura llamada `Mensaje`, que contiene:

- `tipo`: identifica el tipo de mensaje.
- `pais`: identifica el país del agente.
- `id_sector`: identifica el sector de la encuesta.
- `sentimiento`: contiene el valor de sentimiento de la encuesta.

Los tipos de mensajes utilizados son:

| Mensaje | Función |
|---|---|
| `MP_CONEXION` | Informa que un agente se ha conectado. |
| `MP_ENCUESTA` | Envía los datos de una encuesta. |
| `MP_FIN` | Informa que el agente terminó de enviar sus encuestas. |

La estructura compartida y las constantes del proyecto se encuentran en el archivo `COMUN.h`.


## 6. Archivos del proyecto

| Archivo | Descripción |
|---|---|
| `COMUN.h` | Contiene las constantes, estructuras y definiciones compartidas. |
| `generadorCSV.c` | Genera los archivos CSV de encuestas. |
| `agentePais.c` | Lee y envía las encuestas al servidor. |
| `servidorISE.c` | Recibe y procesa las encuestas. |
| `Makefile` | Automatiza la compilación de los programas. |


## Integrantes del grupo
- Santiago Arias Tovar
- Juan Sebastian Quintero Gil
- Jesus Daniel Duque Aparicio
- Pablo Samuel Leon Hernandez
- Katheryn Camacho Gonzalez
