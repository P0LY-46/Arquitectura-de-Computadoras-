#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define MASCARA_LEDS 0x07            // PB0, PB1, PB2
#define MASCARA_INTERRUPTORES 0x07   // PD0, PD1, PD2
#define PIN_INTERRUPTOR_VALOR_1 PD2  // Interruptor 1 (vale 1)
#define PIN_INTERRUPTOR_VALOR_2 PD1  // Interruptor 2 (vale 2)
#define PIN_INTERRUPTOR_VALOR_4 PD0  // Interruptor 3 (vale 4)
#define TOTAL_PASOS_SECUENCIA 4

//DATOS

// Secuencia del pendulo: LED1, LED2, LED3, LED2, LED1, LED2, LED3, LED2
const uint8_t secuencia_leds[TOTAL_PASOS_SECUENCIA] = {
    0x01, 0x02, 0x04, 0x02
};

//PROTOTIPOS
void configurar_puertos(void);
uint8_t leer_numero_interruptores(void);
uint16_t leer_tiempo_paso_ms(void);
void mostrar_leds(uint8_t patron);
void esperar_paso(void);

int main(void) {
    uint8_t paso_actual = 0;

    configurar_puertos();

    while (1) {
        mostrar_leds(secuencia_leds[paso_actual]);
        esperar_paso();

        paso_actual++;
        if (paso_actual >= TOTAL_PASOS_SECUENCIA) {
            paso_actual = 0;
        }
    }

    return 0;
}

//IMPLEMENTACION

void configurar_puertos(void) {
    // PB0..PB2 como salidas (LEDs), apagados al inicio
    DDRB |= MASCARA_LEDS;
    PORTB &= ~MASCARA_LEDS;

    // PD0..PD2 como entradas con pull-up interno.
    // Los interruptores van a tierra: abierto = 1, cerrado (ON) = 0
    DDRD &= ~MASCARA_INTERRUPTORES;
    PORTD |= MASCARA_INTERRUPTORES;
}

uint8_t leer_numero_interruptores(void) {
    // Se invierte PIND para que interruptor en ON = 1.
    // El numero se arma sumando el valor de cada interruptor encendido,
    // asi que el resultado es de 0 a 7
    uint8_t lectura = (uint8_t)(~PIND);
    uint8_t numero = 0;

    if (lectura & (1 << PIN_INTERRUPTOR_VALOR_1)) {
        numero += 1;
    }
    if (lectura & (1 << PIN_INTERRUPTOR_VALOR_2)) {
        numero += 2;
    }
    if (lectura & (1 << PIN_INTERRUPTOR_VALOR_4)) {
        numero += 4;
    }

    return numero;
}

uint16_t leer_tiempo_paso_ms(void) {
    uint16_t tiempo_paso_ms;

    // Cada combinacion de interruptores es unas 1.4 veces mas rapida
    // que la anterior
    switch (leer_numero_interruptores()) {
        case 0: tiempo_paso_ms = 1000; break; // 000 mas lento
        case 1: tiempo_paso_ms = 700;  break; // 001
        case 2: tiempo_paso_ms = 500;  break; // 010
        case 3: tiempo_paso_ms = 350;  break; // 011
        case 4: tiempo_paso_ms = 250;  break; // 100
        case 5: tiempo_paso_ms = 175;  break; // 101
        case 6: tiempo_paso_ms = 125;  break; // 110
        case 7: tiempo_paso_ms = 90;   break; // 111 mas rapido
        default: tiempo_paso_ms = 1000; break;
    }

    return tiempo_paso_ms;
}

void mostrar_leds(uint8_t patron) {
    PORTB = (PORTB & ~MASCARA_LEDS) | (patron & MASCARA_LEDS);
}

void esperar_paso(void) {
    uint16_t milisegundos_transcurridos = 0;

    // _delay_ms necesita una constante, por eso se espera de 1 ms en 1 ms.
    // El tiempo se vuelve a leer en cada vuelta: si cambias los interruptores
    // a media espera, el cambio se nota de inmediato
    while (milisegundos_transcurridos < leer_tiempo_paso_ms()) {
        _delay_ms(1);
        milisegundos_transcurridos++;
    }
}