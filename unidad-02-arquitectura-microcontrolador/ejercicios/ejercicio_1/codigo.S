#include <avr/io.h>
.section .text
.global main

main:

MAIN_LOOP:
	ldi R16, 0x0A
	mov R17, R16
	rjmp MAIN_LOOP
	
