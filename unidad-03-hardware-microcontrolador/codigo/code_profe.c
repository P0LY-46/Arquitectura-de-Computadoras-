// Blink de un LED (PB5 = pin 13 en Arduino UNO) cada 500 ms usando el Timer1 en modo CTC.

#define F_CPU 16000000UL    
#include <avr/io.h>             // Nombres de registros y pines (DDRB, PORTB, TCCR1A...)
#include <avr/interrupt.h>      // Macros de interrupciones: ISR(), sei()
#include <stdint.h>             // Tipos de tamano fijo (uint16_t) es el que nos ayudara a poder usar el volantile 

// el prefijo volatile se usa para la variable se quede sin modificar ya que usualmente cambia dentro de una interrupcion, asi el compilador no la optimiza
volatile uint16_t contador_ms = 0;   // Cuenta cuantos milisegundos han pasado

// Rutina de macro de interrupcion: se ejecuta cada vez que TCNT1 == OCR1A (Comparamos Match A del Timer1) 
ISR(TIMER1_COMPA_vect)        
{
    contador_ms++;              // Llego 1 ms mas (la interrupcion ocurre cada 1 ms)
    if (contador_ms >= 500)     //evalua si ya llego a 500
    {
        PORTB ^= (1 << PB5);    // si estaba prendido se apaga y viceverza
        contador_ms = 0;        // Reinicia la cuenta para el siguiente ciclo
    }
}                              

int main(void) 
{
    DDRB |= (1 << DDB5);        // PB5 como salida (pone el bit 5 de DDRB en 1)
    TCCR1A = 0;                 // se aterriza el eegistro A en 0: sin PWM ni salidas OC1x
    TCCR1B = 0;                 // Registro B en 0: timer detenido, se configura abajo
    TCNT1  = 0;                 // Contador del timer arranca en 0

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // aqui si esta calculando un prescaler 64: 16 MHz / 64 = 250 kHz -> cada cuenta dura 4 microseg.
    // 250 iteraciones * 4 mucroseg = 1 ms, y como se cuenta desde 0 el tope es 249
    OCR1A = 249;                // Valor de comparacion: al llegar aqui salta la interrupcion lo hacemos de 250 desde 0 hasta el 249

    TCCR1B |= (1 << WGM12);                 // activamos el modo CTC: al igualar OCR1A el contador vuelve a 0;
    TCCR1B |= (1 << CS11) | (1 << CS10);    // el preescalador es de 64 (CS11=1, CS10=1) y arranca el timer 

    TIMSK1 |= (1 << OCIE1A);    // Habilita la interrupcion por comparacion
    sei();                      // Habilita las interrupciones globales

    while (1)                   // Bucle infinito: el parpadeo lo hace la ISR, aqui no hay nada
    {
    }
}

