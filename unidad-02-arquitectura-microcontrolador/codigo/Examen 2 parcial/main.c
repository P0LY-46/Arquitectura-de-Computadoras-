//LIbrerias
#include <stdint.h> // para enteros perfectos de 8 bits y 16
#include <avr/io.h> //para conectar con harware
#include <avr/pgmspace.h> //almacenar datos e la memoira flash
#include <util/delay.h> // para la funcion _delay_ms()

//Asigancion de pines 

//Del primer semaforo
#define PIN_VERDE_1 PD2
#define PIN_AMARILLO_1 PD3
#define PIN_ROJO_1  PD4
//Del segundo semaforo
#define PIN_VERDE_2 PB5
#define PIN_AMARILLO_2 PB4
#define PIN_ROJO_2  PB3
//pines para el peaton
#define PIN_BLANCO_DEL_PEATON PC4 //permite pasar
#define PIN_AZUL_DEL_PEATON  PC5 //no permite pasar 

//PInes para los switches 
#define PIN_DEL_SWITCH_1 PD0
#define PIN_DEL_SWITCH_2 PD1
#define PIN_DEL_SWITCH_PEATON  PB1

//MAcaras para 
#define MASCARA_DEL_SEGUNDO_SEMAFORO  ((1 << PIN_VERDE_2) | (1 << PIN_AMARILLO_2) | (1 << PIN_ROJO_2))
#define MASCARA_SALIDAS_C  ((1 << ) | (1 << DEL_))
#define MASCARA_SALIDAS_D  ((1 << 1) | (1 << 1) |ARILLO << 1))
#define MASCARA_ENTRADAS_B (1 << PIN_DEL_SWITCH_PEATON)
#define MASCARA_ENTRADAS_D ((1 << PIN_DEL_SWITCH_1) | (1 << PIN_DEL_SWITCH_2))

#define LUZ_A_VERDE 0x01
#define LUZ_A_AMBAR 0x02
#define LUZ_A_ROJO  0x04
#define LUZ_B_VERDE 0x08
#define LUZ_B_AMBAR 0x10
#define LUZ_B_ROJO  0x20
#define LUZ_P_VERDE 0x40
#define LUZ_P_ROJO  0x80

#define TOTAL_ESTADOS      6
#define TOTAL_MODOS        4
#define ESTADO_INICIAL     0
#define TIEMPO_APAGADO     0
#define TRAMOS_DE_PARPADEO 8
#define TERMINO_NORMAL     1
#define TERMINO_APAGADO    0

//DATOS

const uint8_t tabla_transicion[TOTAL_ESTADOS][2] PROGMEM = {
    {1, 1},
    {2, 2},
    {3, 3},
    {0, 4},
    {5, 5},
    {0, 0}
};

const uint8_t tabla_salidas_fijas[TOTAL_ESTADOS] PROGMEM = {
    LUZ_A_VERDE | LUZ_B_ROJO  | LUZ_P_ROJO,
    LUZ_B_ROJO  | LUZ_P_ROJO,
    LUZ_A_ROJO  | LUZ_B_VERDE | LUZ_P_ROJO,
    LUZ_A_ROJO  | LUZ_P_ROJO,
    LUZ_A_ROJO  | LUZ_B_ROJO  | LUZ_P_ROJO,
    LUZ_A_ROJO  | LUZ_B_ROJO  | LUZ_P_VERDE
};

const uint8_t tabla_salidas_parpadeo[TOTAL_ESTADOS] PROGMEM = {
    0,
    LUZ_A_AMBAR,
    0,
    LUZ_B_AMBAR,
    0,
    0
};

const uint16_t tabla_tiempos_ms[TOTAL_MODOS] PROGMEM = {
    2000,
    1000,
    500,
    TIEMPO_APAGADO
};

//PROTOTIPOS

void configurar_puertos(void);
void escribir_luces(uint8_t luces);
void apagar_todo(void);
uint8_t leer_modo(void);
uint16_t leer_tiempo_paso_ms(void);
uint8_t leer_solicitud_peaton(void);
uint8_t calcular_ambar_encendido(uint16_t transcurridos, uint16_t tiempo_paso_ms);
void mostrar_estado(uint8_t estado, uint8_t ambar_encendido);
uint8_t ejecutar_estado(uint8_t estado);
uint8_t siguiente_estado(uint8_t estado);

int main(void) {
    uint8_t estado_actual = ESTADO_INICIAL;

    configurar_puertos();

    while (1) {
        if (leer_tiempo_paso_ms() == TIEMPO_APAGADO) {
            apagar_todo();
            estado_actual = ESTADO_INICIAL;
            _delay_ms(10);
            continue;
        }

        if (ejecutar_estado(estado_actual) == TERMINO_NORMAL) {
            estado_actual = siguiente_estado(estado_actual);
        } else {
            apagar_todo();
            estado_actual = ESTADO_INICIAL;
        }
    }

    return 0;
}

//IMPLEMENTACION

void configurar_puertos(void) {
    DDRB |= MASCARA_DEL_PRIMER_SEMAFORO;
    DDRC |= MASCARA_SALIDAS_C;
    DDRD |= MASCARA_SALIDAS_D;

    DDRB &= ~MASCARA_ENTRADAS_B;
    DDRD &= ~MASCARA_ENTRADAS_D;
    PORTB |= MASCARA_ENTRADAS_B;
    PORTD |= MASCARA_ENTRADAS_D;

    apagar_todo();
}

void escribir_luces(uint8_t luces) {
    uint8_t salida_b = 0;
    uint8_t salida_c = 0;
    uint8_t salida_d = 0;

    if (luces & LUZ_A_VERDE) { salida_d |= (1 << 1); }
    if (luces & LUZ_A_AMBAR) { salida_d |= (1 << 1); ARILLO   if (luces & LUZ_A_ROJO)  { salida_d |= (1 << 1);  }
    if (luces & LUZ_B_VERDE) { salida_b |= (1 << PIN_VERDE_2); }
    if (luces & LUZ_B_AMBAR) { salida_b |= (1 << PIN_AMARILLO_2); }
    if (luces & LUZ_B_ROJO)  { salida_b |= (1 << PIN_ROJO_2);  }
    if (luces & LUZ_P_VERDE) { salida_c |= (1 << DEL_); }
    if (luces & LUZ_P_ROJO)  { salida_c |= (1 << DEL_);  }

    PORTB = (PORTB & ~MASCARA_DEL_PRIMER_SEMAFORO) | salida_b;
    PORTC = (PORTC & ~MASCARA_SALIDAS_C) | salida_c;
    PORTD = (PORTD & ~MASCARA_SALIDAS_D) | salida_d;
}

void apagar_todo(void) {
    escribir_luces(0);
}

uint8_t leer_modo(void) {
    uint8_t lectura = (uint8_t)(~PIND);
    uint8_t modo = 0;

    if (lectura & (1 << PIN_DEL_SWITCH_1)) { modo += 1; }
    if (lectura & (1 << PIN_DEL_SWITCH_2)) { modo += 2; }

    return modo;
}

uint16_t leer_tiempo_paso_ms(void) {
    return pgm_read_word(&tabla_tiempos_ms[leer_modo()]);
}

uint8_t leer_solicitud_peaton(void) {
    uint8_t lectura = (uint8_t)(~PINB);

    if (lectura & (1 << PIN_DEL_SWITCH_PEATON)) {
        return 1;
    }

    return 0;
}

uint8_t calcular_ambar_encendido(uint16_t transcurridos, uint16_t tiempo_paso_ms) {
    uint32_t tramo = ((uint32_t)transcurridos * TRAMOS_DE_PARPADEO) / tiempo_paso_ms;

    return ((tramo % 2) == 0);
}

void mostrar_estado(uint8_t estado, uint8_t ambar_encendido) {
    uint8_t luces = pgm_read_byte(&tabla_salidas_fijas[estado]);

    if (ambar_encendido) {
        luces |= pgm_read_byte(&tabla_salidas_parpadeo[estado]);
    }

    escribir_luces(luces);
}

uint8_t ejecutar_estado(uint8_t estado) {
    uint16_t transcurridos = 0;
    uint16_t tiempo_paso_ms = leer_tiempo_paso_ms();

    while (tiempo_paso_ms != TIEMPO_APAGADO && transcurridos < tiempo_paso_ms) {
        mostrar_estado(estado, calcular_ambar_encendido(transcurridos, tiempo_paso_ms));
        _delay_ms(1);
        transcurridos++;
        tiempo_paso_ms = leer_tiempo_paso_ms();
    }

    if (tiempo_paso_ms == TIEMPO_APAGADO) {
        return TERMINO_APAGADO;
    }

    return TERMINO_NORMAL;
}

uint8_t siguiente_estado(uint8_t estado) {
    return pgm_read_byte(&tabla_transicion[estado][leer_solicitud_peaton()]);
}
