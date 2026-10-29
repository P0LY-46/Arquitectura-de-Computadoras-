#ifndef F_CPU
#define F_CPU 16000000UL // Frecuencia del reloj (ajustar si es distinta)
#endif

#include <avr/io.h>     // Librería para registros de I/O
#include <util/delay.h> // Librería de AVR para retardos temporales

// Muestra un resultado de 4 bits en los LEDs (PC0..PC3)
// sin tocar los demás pines del puerto C
static void mostrar_leds(uint8_t valor) {
    PORTC = (PORTC & 0xF0) | (valor & 0x0F);
}

int main(void) {
    // PC0..PC3 como salidas (LEDs)
    DDRC |= 0x0F;
    PORTC &= 0xF0; // LEDs apagados al inicio

    // PD0..PD7 como entradas (DIP switch)
    DDRD = 0x00;
    // Los switches van a tierra, así que se activan las resistencias
    // pull-up internas: switch abierto = 1, switch cerrado (ON) = 0
    PORTD = 0xFF;

    while (1) {
        // Leer los 8 switches. Se invierte para que ON = 1
        uint8_t dip = ~PIND;

        uint8_t nibble1 = dip & 0x0F;         // Switches 1-4 (PD0..PD3)
        uint8_t nibble2 = (dip >> 4) & 0x0F;  // Switches 5-8 (PD4..PD7)

        // Operación AND
        mostrar_leds(nibble1 & nibble2);
        _delay_ms(500);

        // Operación OR
        mostrar_leds(nibble1 | nibble2);
        _delay_ms(500);

        // Operación XOR
        mostrar_leds(nibble1 ^ nibble2);
        _delay_ms(500);
    }

    return 0;
}
