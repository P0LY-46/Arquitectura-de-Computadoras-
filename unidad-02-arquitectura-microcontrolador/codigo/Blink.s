#include <avr/io.h>
.equ LED_PIN, 0

.section .text
.global main

main:
    ; ----- Configurar PB5 como salida -----
    sbi _SFR_IO_ADDR(DDRB), LED_PIN

MAIN_LOOP:
    ; ----- Encender LED -----
    sbi _SFR_IO_ADDR(PORTB), LED_PIN
    rcall DELAY

    ; ----- Apagar LED -----
    cbi _SFR_IO_ADDR(PORTB), LED_PIN
    rcall DELAY

    rjmp MAIN_LOOP

; ====================================================
;  Subrutina de retardo
; ====================================================
DELAY:
    ldi R18, 0x20
DELAY1:
    ldi R19, 0xFF
DELAY2:
    ldi R20, 0xFF
DELAY3:
    subi R20, 1
    brne DELAY3
    subi R19, 1
    brne DELAY2
    subi R18, 1
    brne DELAY1
    ret
