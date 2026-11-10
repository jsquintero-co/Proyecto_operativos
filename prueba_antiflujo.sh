#!/bin/bash
#/*#################################
#*   Pontificia Universidad Javeriana
#*   Tema: Plan de Pruebas de Antiflujo y Robustez (Script Shell)
#*   Proyecto ISE
###################################*/

echo "=========================================================="
echo "    INICIANDO PLAN DE PRUEBAS DE ANTIFLUJO Y ROBUSTEZ    "
echo "=========================================================="
echo ""

# Variables para llevar el conteo de pruebas
PRUEBAS_PASADAS=0
PRUEBAS_TOTALES=0
PIPE_TEST="fifo_antiflujo"

# Función auxiliar para registrar resultados
evaluar_prueba() {
    local num_prueba="$1"
    local desc_prueba="$2"
    local codigo_esperado="$3"
    local codigo_obtenido="$4"

    PRUEBAS_TOTALES=$((PRUEBAS_TOTALES + 1))
    echo -n "Prueba $num_prueba: $desc_prueba -> "

    if [ "$codigo_obtenido" -eq "$codigo_esperado" ]; then
        echo "PASÓ (Código retornado: $codigo_obtenido)"
        PRUEBAS_PASADAS=$((PRUEBAS_PASADAS + 1))
    else
        echo "FALLÓ (Esperado: $codigo_esperado, Obtenido: $codigo_obtenido)"
    fi
}

# 1. Preparación previa
echo "[1/2] Limpiando y compilando el proyecto..."
make clean > /dev/null 2>&1
rm -f ${PIPE_TEST} encuestas_*.csv archivo_invalido.csv archivo_vacio.csv
make > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "ERROR: Falló la compilación del proyecto."
    exit 1
fi
echo "     -> Compilación exitosa."
echo ""

echo "[2/2] Ejecutando Casos de Antiflujo..."
echo "----------------------------------------------------------"

# --- PRUEBA 1: Servidor con parámetro de tiempo inválido (< 30s) ---
./servidorISE -b 10 -p ${PIPE_TEST} -t 15 -a 2 > /dev/null 2>&1
evaluar_prueba "1" "Servidor rechaza tiempo inferior a 30s (-t 15)" 1 $?

# --- PRUEBA 2: Servidor con parámetro de tiempo inválido (> 120s) ---
./servidorISE -b 10 -p ${PIPE_TEST} -t 200 -a 2 > /dev/null 2>&1
evaluar_prueba "2" "Servidor rechaza tiempo superior a 120s (-t 200)" 1 $?

# --- PRUEBA 3: Agente con archivo CSV inexistente ---
./agentePais -n archivo_que_no_existe.csv -p ${PIPE_TEST} > /dev/null 2>&1
evaluar_prueba "3" "Agente detecta archivo inexistente" 1 $?

# --- PRUEBA 4: Agente con archivo totalmente vacío ---
touch archivo_vacio.csv
./agentePais -n archivo_vacio.csv -p ${PIPE_TEST} > /dev/null 2>&1
evaluar_prueba "4" "Agente detecta archivo CSV vacío" 1 $?

# --- PRUEBA 5: Agente con archivo con formato corrupto/inválido ---
echo "Encuesta,Pais,Sector,Sentimiento" > archivo_invalido.csv
echo "TextoInvalidoSinEstructura" >> archivo_invalido.csv
./agentePais -n archivo_invalido.csv -p ${PIPE_TEST} > /dev/null 2>&1
evaluar_prueba "5" "Agente rechaza archivo con formato corrupto" 1 $?

# --- PRUEBA 6: Generador con país no válido ---
./generadorCSV -p XX -e 50 > /dev/null 2>&1
evaluar_prueba "6" "Generador rechaza país no válido (-p XX)" 1 $?

# --- PRUEBA 7: Generador con porcentaje fuera de rango (> 100%) ---
./generadorCSV -p FR -e 150 > /dev/null 2>&1
evaluar_prueba "7" "Generador rechaza porcentaje superior a 100% (-e 150)" 1 $?

# --- PRUEBA 8: Agente intenta conectar a un Pipe Nominal no creado ---
./agentePais -n encuestas_FR.csv -p pipe_inexistente > /dev/null 2>&1
evaluar_prueba "8" "Agente falla si el Pipe Nominal no existe" 1 $?

# --- PRUEBA 9: Ejecución con flags incompletos en Agente ---
./agentePais -n encuestas_FR.csv > /dev/null 2>&1
evaluar_prueba "9" "Agente detecta falta de flag -p" 1 $?

# Limpieza final de archivos generados durante las pruebas
rm -f ${PIPE_TEST} encuestas_*.csv archivo_invalido.csv archivo_vacio.csv

echo "----------------------------------------------------------"
echo ""
echo "=========================================================="
echo "                 RESUMEN FINAL DE PRUEBAS                 "
echo "=========================================================="
echo " Pruebas Pasadas: $PRUEBAS_PASADAS / $PRUEBAS_TOTALES"

PORCENTAJE=$(( (PRUEBAS_PASADAS * 100) / PRUEBAS_TOTALES ))
echo " Tasa de Éxito:   ${PORCENTAJE}%"

if [ "$PRUEBAS_PASADAS" -eq "$PRUEBAS_TOTALES" ]; then
    echo " RESULTADO: ¡TODAS LAS PRUEBAS DE ANTIFLUJO PASARON CON ÉXITO!"
else
    echo " RESULTADO: Se detectaron fallos en algunas pruebas de antiflujo."
fi
echo "=========================================================="
