#include <avr/io.h>

.section .text
.global main

main:
    ; 1. Configurar LEDs (PB0, PB1, PB2) como salidas
    ldi r16, 0x07
    out _SFR_IO_ADDR(DDRB), r16

    ; 2. Configurar Switches (PC0, PC1) como entradas
    ldi r16, 0x00
    out _SFR_IO_ADDR(DDRC), r16      ; Puerto C como entrada

    ; 3. Activar resistencias Pull-up en PC0 y PC1 para que lean '1' al estar abiertos
    ldi r16, 0b00000011
    out _SFR_IO_ADDR(PORTC), r16

MAIN_LOOP:
    ; --- Paso 1: Encender D1 (PB0) ---
    ldi r16, 0b00000001
    out _SFR_IO_ADDR(PORTB), r16
    rcall LEER_SWITCHES_Y_ESPERAR

    ; --- Paso 2: Encender D2 (PB1) ---
    ldi r16, 0b00000010
    out _SFR_IO_ADDR(PORTB), r16
    rcall LEER_SWITCHES_Y_ESPERAR

    ; --- Paso 3: Encender D3 (PB2) ---
    ldi r16, 0b00000100
    out _SFR_IO_ADDR(PORTB), r16
    rcall LEER_SWITCHES_Y_ESPERAR

    ; --- Paso 4: Regresar a D2 (PB1) ---
    ldi r16, 0b00000010
    out _SFR_IO_ADDR(PORTB), r16
    rcall LEER_SWITCHES_Y_ESPERAR

    rjmp MAIN_LOOP                     ; Repetir el péndulo infinito


; ====================================================================
; SUBRUTINA: Lee los switches en PC0/PC1, define velocidad y hace pausa
; ====================================================================
LEER_SWITCHES_Y_ESPERAR:
    ; Leer todo el Puerto C (PINC)
    in r17, _SFR_IO_ADDR(PINC)

    ; Mascara: Borrar todos los bits excepto PC0 y PC1 (0b00000011)
    andi r17, 0b00000011

    ; --- Evaluar Estado 00 (Ambos cerrados a tierra) -> Mas rapido ---
    cpi r17, 0b00000000
    breq ESTADO_00

    ; --- Evaluar Estado 01 (PC0 abierto, PC1 cerrado) -> Medio-lento ---
    ; PC0 es el bit 0 (00000001 en binario)
    cpi r17, 0b00000001
    breq ESTADO_01

    ; --- Evaluar Estado 10 (PC0 cerrado, PC1 abierto) -> Medio-rapido ---
    ; PC1 es el bit 1 (00000010 en binario)
    cpi r17, 0b00000010
    breq ESTADO_10

    ; --- Si no fue ninguno, es Estado 11 (Ambos abiertos) -> Mas lento ---
    rjmp ESTADO_11

; --- Asignacion de velocidades a r18 ---
ESTADO_00:
    ldi r18, 0x05 ; Numero mas chico = mas rapido
    rjmp EJECUTAR_DELAY

ESTADO_01:
    ldi r18, 0x25 ; Medio-lento
    rjmp EJECUTAR_DELAY

ESTADO_10:
    ldi r18, 0x12 ; Medio-rapido
    rjmp EJECUTAR_DELAY

ESTADO_11:
    ldi r18, 0x60    ; Numero mas grande = mas lento

EJECUTAR_DELAY:
    ; Inicia el Delay original con la velocidad ajustada
DELAY1:
    ldi r19, 0xFF
DELAY2:
    ldi r20, 0xFF
DELAY3:
    dec r20
    brne DELAY3
    dec r19
    brne DELAY2
    dec r18
    brne DELAY1
    ret
