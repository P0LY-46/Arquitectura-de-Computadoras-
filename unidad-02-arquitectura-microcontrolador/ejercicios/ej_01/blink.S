#include <avr/io.h>

.global main
main:  
	sbi _SFR_IO_ADDR(DDRB), PB5 ; Configura el PIN 13 (LED) como salida 
loop: 
	sbi _SFR_IO_ADDR(PORTB), PB5 ; Enciende el LED
	rjmp loop
