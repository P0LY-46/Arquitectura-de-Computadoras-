;==========================================================
; pendulo.asm - AVRASM2 para ATmega328P físico / Proteus
;==========================================================
.include "m328Pdef.inc"

.cseg
.org 0x0000
rjmp RESET

RESET:
    ; Configurar PB0, PB1 y PB2 como salidas (0b00000111 = 0x07)
    ldi r16, 0x07
    out DDRB, r16

MAIN_LOOP:
    ; Paso 1: Encender D1 (PB0)
    ldi r16, 0b00000001
    out PORTB, r16
    rcall DELAY

    ; Paso 2: Encender D2 (PB1)
    ldi r16, 0b00000010
    out PORTB, r16
    rcall DELAY

    ; Paso 3: Encender D3 (PB2)
    ldi r16, 0b00000100
    out PORTB, r16
    rcall DELAY

    ; Paso 4: Regresar a D2 (PB1) para completar la oscilacion del pendulo
    ldi r16, 0b00000010
    out PORTB, r16
    rcall DELAY

    rjmp MAIN_LOOP

; Subrutina de retardo limpia
DELAY:
    ldi r18, 0x20
DELAY1:
    ldi r19, 0xFF
DELAY2:
    ldi r20, 0xFF
DELAY3:
    dec r20            ; Equivale a subi r20, 1
    brne DELAY3
    dec r19
    brne DELAY2
    dec r18
    brne DELAY1
    ret
