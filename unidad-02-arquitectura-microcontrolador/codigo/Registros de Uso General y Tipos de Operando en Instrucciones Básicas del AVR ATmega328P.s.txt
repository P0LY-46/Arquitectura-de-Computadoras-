#include <avr/io.h>

.section .text
.global main

main:
    ldi r16, VALOR_1      ; operando 1
    ldi r17, VALOR_2      ; operando 2
    add r16, r17          ; suma y guarda en r16
MAIN_LOOP:
    rjmp MAIN_LOOP        ; bucle infinito
