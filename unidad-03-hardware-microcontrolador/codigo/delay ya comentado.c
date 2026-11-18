
#define F_CPU 16000000UL
#include <avr/io.h>             // Nombres de registros y pines (DDRB, PORTB, TCCR1A...)
#include <avr/interrupt.h>      // Macros de interrupciones: ISR(), sei()
#include <stdint.h>             // Tipos de tamano fijo (uint16_t) es el que nos ayudara a poder usar el volantile

// el prefijo volatile se usa para la variable se quede sin modificar ya que usualmente cambia dentro de una interrupcion, asi el compilador no la optimiza
volatile uint16_t contador_ms = 0;   // Cuenta cuantos milisegundos han pasado
volatile uint16_t limite_ms = 0;     // Tiempo entre cambios del LED segun la velocidad elegida (0 = apagado)

// Rutina de macro de interrupcion: se ejecuta cada vez que TCNT1 == OCR1A (Comparamos Match A del Timer1)
ISR(TIMER1_COMPA_vect)
{
    if (limite_ms == 0)             // limite 0 = modo apagado (switches en 00)
    {
        PORTB &= ~(1 << PB5);       // Fuerza el LED apagado
        contador_ms = 0;
        return;
    }
    contador_ms++;                  // Llego 1 ms mas (la interrupcion ocurre cada 1 ms)
    if (contador_ms >= limite_ms)   // evalua si ya llego al limite elegido por los switches
    {
        PORTB ^= (1 << PB5);        // si estaba prendido se apaga y viceversa
        contador_ms = 0;            // Reinicia la cuenta para el siguiente ciclo
    }
}

int main(void)
{
    DDRB |= (1 << DDB5);                        // PB5 como salida (pone el bit 5 de DDRB en 1)
    DDRD &= ~((1 << DDD0) | (1 << DDD1));       // PD0 y PD1 como entradas
    PORTD |= (1 << PORTD0) | (1 << PORTD1);     // Activa las resistencias pull-up internas

    TCCR1A = 0;                 // se aterriza el registro A en 0: sin PWM ni salidas OC1x
    TCCR1B = 0;                 // Registro B en 0: timer detenido, se configura abajo
    TCNT1  = 0;                 // Contador del timer arranca en 0

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // aqui si esta calculando un prescaler 64: 16 MHz / 64 = 250 kHz -> cada cuenta dura 4 microseg.
    // 250 iteraciones * 4 microseg = 1 ms, y como se cuenta desde 0 el tope es 249
    OCR1A = 249;                // Valor de comparacion: al llegar aqui salta la interrupcion lo hacemos de 250 desde 0 hasta el 249

    TCCR1B |= (1 << WGM12);                 // activamos el modo CTC: al igualar OCR1A el contador vuelve a 0;
    TCCR1B |= (1 << CS11) | (1 << CS10);    // el preescalador es de 64 (CS11=1, CS10=1) y arranca el timer

    TIMSK1 |= (1 << OCIE1A);    // Habilita la interrupcion por comparacion
    sei();                      // Habilita las interrupciones globales

    while (1)                   // Bucle infinito: el parpadeo lo hace la ISR, aqui se leen los switches
    {
        // Con pull-up, switch cerrado = 0 en el pin, por eso se invierte (cerrado = valor activo)
        uint8_t sw1 = (PIND & (1 << PIND0)) ? 0 : 1;    // PD0 -> aporta 0 o 1
        uint8_t sw2 = (PIND & (1 << PIND1)) ? 0 : 2;    // PD1 -> aporta 0 o 2
        uint8_t velocidad = sw2 | sw1;                  // 0, 1, 2 o 3

        // Tabla de velocidades:
        //   SW2 SW1 | decimal | velocidad
        //    0   0  |    0    | LED apagado
        //    0   1  |    1    | lento  (500 ms)
        //    1   0  |    2    | medio  (250 ms)
        //    1   1  |    3    | rapido (100 ms)
        switch (velocidad)
        {
            case 1:  limite_ms = 500;  break;           // 01 = lento
            case 2:  limite_ms = 250;  break;           // 10 = medio
            case 3:  limite_ms = 100;  break;           // 11 = rapido
            default: limite_ms = 0;    break;           // 00 = LED apagado (la ISR lo interpreta asi)
        }
    }
}
