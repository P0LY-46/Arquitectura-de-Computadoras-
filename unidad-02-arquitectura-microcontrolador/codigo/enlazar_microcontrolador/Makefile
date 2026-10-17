# Variables 
MCU = atmega328p
TARGET = main
BUILD_DIR = build

# Regla principal
all: $(BUILD_DIR)/$(TARGET).elf

# Crear carpeta build y compilar 
$(BUILD_DIR)/$(TARGET).elf: main.s
	mkdir -p $(BUILD_DIR)
	avr-gcc -mmcu$(MCU) -g -nostdlib -o $@ $<

clean: 
	rm -rf $(BUILD_DIR)
