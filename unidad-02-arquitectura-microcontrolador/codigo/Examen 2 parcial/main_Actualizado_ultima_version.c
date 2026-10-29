//LIbrerias
#include <stdint.h> // para enteros perfectos de 8 bits y 16
#include <avr/io.h> //para conectar con harware
#include <avr/pgmspace.h> //almacenar datos e la memoira flash
#include <util/delay.h> // para la funcion _delay_ms(
//Asigancion de pines 
//Del primer semaforo
#define PIN_VERDE_1 PD2
#define PIN_AMARILLO_1 PD3
#define PIN_ROJO_1  PD4
//Del segundo semaforo
#define PIN_VERDE_2 PB5
#define PIN_AMARILLO_2 PB4
#define PIN_ROJO_2  PB3
//pines para el peaton
#define PIN_BLANCO_DEL_PEATON PC4 //permite pasar
#define PIN_AZUL_DEL_PEATON  PC5 //no permite pasar 
//PInes para los switches 
#define PIN_DEL_SWITCH_1 PD0 
#define PIN_DEL_SWITCH_2 PD1 
#define PIN_DEL_SWITCH_PEATON  PB1
//MAcaras para 
// Union de los bits 5z, 4 y 3 las cuales van a ser salidas 
#define MASCARA_DEL_SEGUNDO_SEMAFORO  ((1 << PIN_VERDE_2) | (1 << PIN_AMARILLO_2) | (1 << PIN_ROJO_2)) //Union de los pines de Salida del segundo semaforo
// Union de los bits PC5 Y PC4 las cuales van a ser salidas
#define MASCARA_SALIDA_DEL_PEATON  ((1 << PIN_BLANCO_DEL_PEATON) | (1 << PIN_AZUL_DEL_PEATON)) //Union de los pines de pase al peaton
// Union de los bits 2, 3 y 4 las cuales van a ser salidas 
#define MASCARA_DEL_PRIMER_SEMAFORO  ((1 << PIN_VERDE_1) | (1 << PIN_AMARILLO_1) | (1 << PIN_ROJO_1)) // Union de los pines de salida del primer semaforo

#define MASCARA_ENTRADA_SWITCH_PEATON (1 << PIN_DEL_SWITCH_PEATON) // dar de alta pin del switch

// Union de los bits PD0 Y PD1 para los switches del control de velocidades
#define MASCARA_ENTRADAS_SWITCH_VELOCIDADES ((1 << PIN_DEL_SWITCH_1) | (1 << PIN_DEL_SWITCH_2)) 

//Nombre de las luces (Bloque de 8 bits)
#define LUZ_VERDE_1 0x01
#define LUZ_AMARILLA_1 0x02
#define LUZ_ROJA_1  0x04
#define LUZ_VERDE_2 0x08
#define LUZ_AMARILLA_2 0x10
#define LUZ_ROJA_2  0x20
#define LUZ_BLANCA_PARA_PEATON 0x40
#define LUZ_AZUL_PARA_PEATON  0x80

// constantes de limites de control
#define TOTAL_ESTADOS      6 //total de todos los estados 
#define CANTIDAD_DE_VELOCIDADES   4 //velocidades o tiempos de ruacion de un segundo 500 iliseg y etc,
#define ESTADO_INICIAL     0 // estado en el que iniciamos 
#define TIEMPO_APAGADO     0 //
#define PEDACITOS_PARA_PARPADEO 8 //segmentos de los cuales separamos para el paradeo 
#define TERMINO_NORMAL     1 // valor si es estado actual normal
#define TERMINO_APAGADO    0 // valor que representa que esta apagado 

//tablas de estado y control 
const uint8_t tabla_de_cambio_de_estados[TOTAL_ESTADOS][2] PROGMEM = { //GUuardamos esto en memria flash
    {1, 1}, {2, 2}, {3, 3},  {0, 4}, {5, 5},  {0, 0}  
// Estado0, Estado1,Estado2, Estado3,Estado4, Estado5,

//  columna 0-> lo que se debe de ahcer si NO encendimos el boton del peaton 
//  Columna 1 -> al estado que debemos de cambiar si SI se encencio el boton de peaton
};

const uint8_t tabla_de_salidas_de_luces_estaticas[TOTAL_ESTADOS] PROGMEM = { // que luces deben estar enscendidas al mismo tiempo del semaoforo 1
    LUZ_VERDE_1 | LUZ_ROJA_2  | LUZ_AZUL_PARA_PEATON,
    LUZ_ROJA_2  | LUZ_AZUL_PARA_PEATON, // aqui es donde parpadea la amarilla
    LUZ_ROJA_1  | LUZ_VERDE_2 | LUZ_AZUL_PARA_PEATON,
    LUZ_ROJA_1  | LUZ_AZUL_PARA_PEATON, //aqui es donde va la luz amarilla del semaoforo 2
    LUZ_ROJA_1  | LUZ_ROJA_2  | LUZ_AZUL_PARA_PEATON,
    LUZ_ROJA_1  | LUZ_ROJA_2  | LUZ_BLANCA_PARA_PEATON
    // aqui vinen las que no parpadean 
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t tabla_de_salidas_de_luces_con_parpadeo[TOTAL_ESTADOS] PROGMEM = { // luces que parpadean de la luz amarilla
    0,
    LUZ_AMARILLA_1, //indicamos que debe estar parpadeando la luz amarilla en este estado 
    0,
    LUZ_AMARILLA_2, // aqui igualo
    0,//en los estado sen cero ninguna luz parpadea
    0
};

const uint16_t tabla_tiempos_ms[CANTIDAD_DE_VELOCIDADES] PROGMEM = { //Tambla de tiempos de la duracion de cada estado 
    2000, //milisegundos
    1000, //milisegundos
    500, //medio segundo
    TIEMPO_APAGADO // aqui se considera como que esta apagado, que la variable la declaramos arriba
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////PARTE DE GABO/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void configurar_puertos(void) { // funcion que se ejecuta una sola vez al inicio para dejar cada pin como entrada o salida
    // Escojemos que tiene que hacer cada pin, si recibir energia o sacar energia
    //Si son las lucecitas -> sacar corriente
    DDRB |= MASCARA_DEL_SEGUNDO_SEMAFORO; // asi asignamos como salida al concatenado de leds del semaforo 2 se usa esta nomenclarura para que altere los demas pines
    DDRC |= MASCARA_SALIDA_DEL_PEATON; // igualmente se le asignan como salidas el concatenado de los 2 leds del mi semaforo del peaton igualmente solo se le asigna como salida a los leds usados
    DDRD |= MASCARA_DEL_PRIMER_SEMAFORO; //igualmente se le asigna a l puerto D salida al concat del semaforo

    //aqui pasa algo interesante y es que tenemos que asignar como entradas por eso se divide en 2 pasos;

    //primero es super importante decir que el signo ~ invierte o los hace negativos, es decir si es (00011) ~(11100) para lo que sea 0 se ponga como 1 se prepare para el pull up
    DDRB &= ~MASCARA_ENTRADA_SWITCH_PEATON; //Con &= configuramos como entrada al pin donde se conecta el switch del paso del peaton, modificando solo ese pin y los demas se quedan talcual
    DDRD &= ~MASCARA_ENTRADAS_SWITCH_VELOCIDADES; //igual que el anterior asignamos como entrada al conjunto de pines a donde van los switches

    //como ya los asignamos como entradasm ahora se necesita configutar nuestro sistema para hacer un pull-up los pines
    PORTB |= MASCARA_ENTRADA_SWITCH_PEATON; //igual que arriba solo modifica los pines seleccionados
    PORTD |= MASCARA_ENTRADAS_SWITCH_VELOCIDADES; // activamos la resistencia pull-up interna en los pines de los switches de velocidad (el pin queda en 1 y al apretar el switch se va a 0)

    apagar_todo(); //reiniciamos todo, que empiecen apagados por la posible basura que puede tener cuand
}
void escribir_luces(uint8_t luces) { // con esto indicamos que luces deben prenderse
    uint8_t salida_b = 0; // aqui vamos armando los bits que se van a prender en el puerto B (empieza en 0 = todo apagado)
    uint8_t salida_c = 0; // aqui vamos armando los bits que se van a prender en el puerto C
    uint8_t salida_d = 0; // aqui vamos armando los bits que se van a prender en el puerto D

    // Revisamos una por una las luces que nos pidieron: con & checamos si ese bit esta prendido en "luces",
    // y si si, con |= prendemos el pin real que le corresponde en el puerto correcto
    if (luces & LUZ_VERDE_1) { salida_d |= (1 << PIN_VERDE_1); } // si piden el verde 1, prendemos el bit de PD2 en la salida del puerto D
    if (luces & LUZ_AMARILLA_1) { salida_d |= (1 << PIN_AMARILLO_1); } // si piden el amarillo 1, prendemos el bit de PD3 en la salida del puerto D
    if (luces & LUZ_ROJA_1)  { salida_d |= (1 << PIN_ROJO_1);  } // si piden el rojo 1, prendemos el bit de PD4 en la salida del puerto D
    if (luces & LUZ_VERDE_2) { salida_b |= (1 << PIN_VERDE_2); } // si piden el verde 2, prendemos el bit de PB5 en la salida del puerto B
    if (luces & LUZ_AMARILLA_2) { salida_b |= (1 << PIN_AMARILLO_2); } // si piden el amarillo 2, prendemos el bit de PB4 en la salida del puerto B
    if (luces & LUZ_ROJA_2)  { salida_b |= (1 << PIN_ROJO_2);  } // si piden el rojo 2, prendemos el bit de PB3 en la salida del puerto B
    if (luces & LUZ_BLANCA_PARA_PEATON) { salida_c |= (1 << PIN_BLANCO_DEL_PEATON); } // si piden la blanca del peaton, prendemos el bit de PC4 en la salida del puerto C
    if (luces & LUZ_AZUL_PARA_PEATON)  { salida_c |= (1 << PIN_AZUL_DEL_PEATON);  } // si piden la azul del peaton, prendemos el bit de PC5 en la salida del puerto C

    // Ahora si mandamos todo a los puertos reales: primero con & ~MASCARA apagamos SOLO los leds de ese grupo
    // (los demas pines como los switches no se tocan) y luego con | ponemos encima los que si deben ir prendidos
    PORTB = (PORTB & ~MASCARA_DEL_SEGUNDO_SEMAFORO) | salida_b; // actualizamos los leds del semaforo 2 en el puerto B
    PORTC = (PORTC & ~MASCARA_SALIDA_DEL_PEATON) | salida_c; // actualizamos los leds del peaton en el puerto C
    PORTD = (PORTD & ~MASCARA_DEL_PRIMER_SEMAFORO) | salida_d; // actualizamos los leds del semaforo 1 en el puerto D
}

void apagar_todo(void) { // funcion para apagar todo
    escribir_luces(0); // le mandamos 0 = ninguna luz pedida, asi que escribir_luces apaga todos los leds
}


uint8_t leer_modo(void) { // lee los 2 switches de velocidad y regresa un numero del 0 al 3 (ese numero es la posicion en la tabla de tiempos)
    uint8_t lectura = (uint8_t)(~PIND); // con ~ invertimos la lectura del puerto
    // (como usamos pull-up, un switch apretado lee 0, entonces al invertir queda en 1 y es mas facil de entender: 1 = activado)
    uint8_t modo = 0; // empezamos suponiendo que ningun switch esta activado

    if (lectura & (1 << PIN_DEL_SWITCH_1)) { modo += 1; } // si el switch 1 esta activado sumamos 1 al modo (bit menos significativo)
    if (lectura & (1 << PIN_DEL_SWITCH_2)) { modo += 2; } // si el switch 2 esta activado sumamos 2 al modo (bit mas significativo)


    return modo; // regresamos el modo: 0 = lento, 1 = medio, 2 = rapido, 3 = apagado
}

uint16_t leer_tiempo_paso_ms(void) { // regresa cuantos milisegundos dura cada estado segun los switches
    return pgm_read_word(&tabla_tiempos_ms[leer_modo()]); // leemos el modo, lo usamos de indice en la tabla y con pgm_read_word sacamos el valor de 16 bits de la memoria flash
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////FIN PARTE DE GABO //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t leer_solicitud_peaton(void) { //revisamos si el boton del peaton esta presionado o no 
   
    //un entero de 8 bits -> retorna un 1 o un 0
    uint8_t lectura = (uint8_t)(~PINB); //leemos las patitas e invertimos el resultado
    //y confirmamos que retorne el resultado como un numero de 8 bits
//  
    if (lectura & (1 << PIN_DEL_SWITCH_PEATON)) {// preguntamos si tiene 1 encendido
        return 1; //si si pues retornamos 1
    }

    return 0; //si no retornamos un cero
}
                                    //tiempo que ha pasado, duracion de tiempo
uint8_t calcular_ambar_encendido(uint16_t transcurridos, uint16_t tiempo_paso_ms) { // verificarf que esta encendido o apagado
    uint32_t tramo = ((uint32_t)transcurridos * PEDACITOS_PARA_PARPADEO) / tiempo_paso_ms;
    //dividimos el tiempo entre 8, y checamos si es par o impar 
    return ((tramo % 2) == 0);
}

void mostrar_estado(uint8_t estado, uint8_t amarillo_encendido) { // aqui es donde juntamos el funcionamiento de los led que parpsdean como los que estan fijos 
    //preguntamos en cual de los estados estaos 
    // prender si es 1 o apagar si es un cero para luz amarilla 
    uint8_t luces = pgm_read_byte(&tabla_de_salidas_de_luces_estaticas[estado]); //llamamos a las luces que no parpadean 

    if (amarillo_encendido) { // preguntamos si el ambar del estado debe de estar encendido si es asi pues llamamos a la funcion que tiene oslo las luces que parpadean 
        luces |= pgm_read_byte(&tabla_de_salidas_de_luces_con_parpadeo[estado]); //
    }// combinar con |= o tambien agregar

    escribir_luces(luces); // y los cambios los actualizamos y se lo enviamos a la funcion de escribir luces
}

uint8_t ejecutar_estado(uint8_t estado) { // aqui es donde tenemos el reloj y actualizamos 
    uint16_t transcurridos = 0; // aqui es donde empezamos el cronometro de nuestro minisistema
    uint16_t tiempo_paso_ms = leer_tiempo_paso_ms(); // aqui es donde leemos el tiempo que debe de durar segun el velocidad

    while (tiempo_paso_ms != TIEMPO_APAGADO && transcurridos < tiempo_paso_ms) { // repite mientras sea diferente de apagado y que sea menor al tiempo del cronometro 
        mostrar_estado(estado, calcular_ambar_encendido(transcurridos, tiempo_paso_ms)); //llamamos a las funciones para ver 
        //si el amarillo debe de estar prendido o apagado
        _delay_ms(1); // y pausa de un milisegundo
        transcurridos++; //y vamos sumando al cronometro
        tiempo_paso_ms = leer_tiempo_paso_ms(); // hemos modificado la velocidad?
    }

    if (tiempo_paso_ms == TIEMPO_APAGADO) { //verificamos si se activo el modo apagado si es asi llamar a la funcion apagado 
        return TERMINO_APAGADO; //regresamos el valor de apagado 
    }

    return TERMINO_NORMAL; // y si no un 1 de normal 
}

uint8_t siguiente_estado(uint8_t estado) { // consyaltmaos la tabla para saber que estado es el siguiente y ver si el peaton pucho o cambio el switch
    return pgm_read_byte(&tabla_de_cambio_de_estados[estado][leer_solicitud_peaton()]);
}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//Funcion principal
int main(void) {
    uint8_t estado_actual = ESTADO_INICIAL; // aqui abrimos un epsacio en la mmeoria que es estado actua y lo inicializamos con el estado inicial 

    configurar_puertos(); // aqui decimos que patas vamos a usar o cuales se van a electrificar 

    while (1) { //nuestro while que es donde hacemos que los etsados se repitan una y otra vez 
        if (leer_tiempo_paso_ms() == TIEMPO_APAGADO) { // si leemos que el estado esta en apagado apagamos todo e incializamos todo 
            apagar_todo(); // llamamos a la funcion de apagar
            estado_actual = ESTADO_INICIAL; // reinciamos e estado a cero o inicial
            _delay_ms(10); // detenemos un ratito el ciclo
            continue; 
        }

        if (ejecutar_estado(estado_actual) == TERMINO_NORMAL) { // Verificamos que el estado actual se haya ejecutado correctamente
            estado_actual = siguiente_estado(estado_actual); //y calculamos cual es el siguiente estado 
        } else {
            apagar_todo(); // si no salio bien apagamos todo 
            estado_actual = ESTADO_INICIAL; // lo reiniciamos
        }
    }

    return 0;
}
