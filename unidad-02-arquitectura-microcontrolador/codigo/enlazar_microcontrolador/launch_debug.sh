#!/bin/bash

ENTORNO="venv"

# 1. Verificar e instalar entorno virtual
if [ -d "$ENTORNO" ]; then
    echo "[-] Entorno de Python ya existe."
else
    echo "[+] El entorno de Python no existe. Creándolo..."
    python3 -m venv venv
    venv/bin/pip install gdbgui
fi

# 2. Compilar el código automáticamente antes de simular
echo "[+] Compilando proyecto..."
make all

# 3. Lanzar simavr en segundo plano (puerto 1234)
echo "[+] Iniciando simavr..."
simavr -g -m atmega328p build/main.elf &
SIMAVR_PID=$! # Guarda el ID del proceso para cerrarlo al final

# Esperar un momento a que simavr ocupe el puerto
sleep 1

# 4. Activar entorno y lanzar GDBGUI
echo "[+] Abriendo GDBGUI..."
source venv/bin/activate
gdbgui -g avr-gdb build/main.elf
deactivate

# 5. Limpieza al cerrar GDBGUI
echo "[+] Cerrando simulador..."
kill $SIMAVR_PID 2>/dev/null
