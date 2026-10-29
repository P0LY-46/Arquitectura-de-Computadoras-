# --- Configuración del Microcontrolador ---
MCU     := atmega328p
F_CPU   := 16000000UL
PROGRAMMER := usbasp

# --- Archivo Fuente (Cambiar a main.S si usas ensamblador) ---
SRC     := main.c

# --- Herramientas ---
CC      := avr-gcc
OBJCOPY := avr-objcopy
AVRDUDE := avrdude

# --- Banderas de Compilación ---
CFLAGS  := -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# Si el archivo fuente es .S, agregamos la bandera para procesar macros e includes
ifeq ($(suffix $(SRC)),.S)
    CFLAGS += -x assembler-with-cpp
endif

DFLAGS  := -ggdb3 -O0

# Target por defecto
all: release

release: $(SRC)
	@echo "Inicializando compilación (Release) desde $(SRC)..."
	mkdir -p release
	$(CC) $(CFLAGS) -Os $(SRC) -o release/main.elf
	$(OBJCOPY) -j .text -j .data -O ihex release/main.elf release/main.hex
	@echo "Compilación exitosa: release/main.hex preparado."

debug: $(SRC)
	@echo "Inicializando compilación modo depuración (Debug) desde $(SRC)..."
	mkdir -p build
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o build/main.elf
	@echo "Compilación exitosa: build/main.elf preparado para GDB."

# Regla para quemar el programa en el microcontrolador usando USBasp
flash: release
	@echo "Cargando firmware mediante USBasp..."
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U flash:w:release/main.hex:i

clean:
	@echo "Limpiando archivos generados..."
	rm -rf build release

.PHONY: all release debug flash clean