#!/bin/bash
#/*#################################
#*   Pontificia Universidad Javeriana
#*   Tema: Plan de Pruebas Automatizado (Script Shell)
#*   Proyecto ISE
###################################*/

echo "=========================================================="
echo "    INICIANDO PLAN DE PRUEBAS AUTOMATIZADO - PROYECTO ISE  "
echo "=========================================================="
echo ""

# 1. Limpieza inicial de ejecutables, datos y pipes previos
echo "[1/5] Limpiando compilaciones y archivos antiguos..."
make clean > /dev/null 2>&1
rm -f ise_fifo encuestas_*.csv
echo "     -> Limpieza completada."
echo ""

# 2. Compilación del proyecto
echo "[2/5] Compilando el proyecto con Makefile..."
make
if [ $? -ne 0 ]; then
    echo "ERROR: Falló la compilación."
    exit 1
fi
echo "     -> Compilación exitosa."
echo ""

# 3. Generación de archivos CSV sintéticos para la prueba
echo "[3/5] Generando datos de prueba (CSV)..."
./generadorCSV -p FR -e 75
./generadorCSV -p ES -e 50
echo "     -> Archivos encuestas_FR.csv y encuestas_ES.csv generados."
echo ""

# 4. Iniciar el Servidor ISE en segundo plano
PIPE_TEST="ise_fifo"
TIEMPO_TEST=35

echo "[4/5] Levantando Servidor Principal ISE (Tiempo: ${TIEMPO_TEST}s, Pipe: ${PIPE_TEST})..."
./servidorISE -b 10 -p ${PIPE_TEST} -t ${TIEMPO_TEST} -a 2 &
SERVER_PID=$!

# Pausa breve para garantizar que el servidor cree el FIFO y quede listo para escuchar
sleep 1

# 5. Ejecutar Agentes de Lectura concurrentemente
echo "[5/5] Lanzando Agentes de Lectura por país..."
echo "     -> Ejecutando Agente Francia (FR)..."
./agentePais -n encuestas_FR.csv -p ${PIPE_TEST} &

echo "     -> Ejecutando Agente España (ES)..."
./agentePais -n encuestas_ES.csv -p ${PIPE_TEST} &

echo ""
echo "----------------------------------------------------------"
echo " Los agentes están transmitiendo. Esperando a que el "
echo " servidor procese las encuestas y finalice (${TIEMPO_TEST}s)..."
echo "----------------------------------------------------------"
echo ""

# Esperar a que el proceso del servidor termine de forma natural
wait $SERVER_PID

echo ""
echo "=========================================================="
echo "          PLAN DE PRUEBAS FINALIZADO CON ÉXITO            "
echo "=========================================================="
