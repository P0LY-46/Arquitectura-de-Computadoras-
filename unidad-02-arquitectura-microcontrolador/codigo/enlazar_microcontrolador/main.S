.section .text
.global main

main:
    ldi r16, 0x7F
    ldi r17, 0x01
    add r16, r17

    ldi r16, 0xFF
    ldi r17, 0x01
    add r16, r17

    ldi r16, 0x10
    ldi r17, 0x20
    add r16, r17

loop:
    rjmp loop
