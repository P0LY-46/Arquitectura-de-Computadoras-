#include <stdio.h>

int main() {

    int cantidad_terminos;

    long long primero = 0, segundo = 1, siguiente;

    printf("Cuantos terminos quieres calcular? ");
    if (scanf("%d", &cantidad_terminos) != 1 || cantidad_terminos <= 0) {
        printf("Por favor ingresa un numero entero positivo.\n");
        return 1;
    }

    printf("Serie de Fibonacci: ");
    for (int i = 1; i <= cantidad_terminos; ++i) {
        printf("%lld", primero);

        if (i < cantidad_terminos) {
            printf(", ");
        }

        siguiente = primero + segundo;
        primero = segundo;
        segundo = siguiente;
    }

    printf("\n");

    return 0;
}
