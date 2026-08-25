#include <stdio.h>
#include <stdlib.h>

int main (){

int contador=0 ,c_numeros=10 ,num_1=0 ,num_2=1 ,num_siguiente;

printf("Serie de Fibonacci \n");
	
	while(contador<=c_numeros){
	
	
		num_siguiente= num_1+num_2;
		num_1=num_2; 
		num_2=num_siguiente;
	
		printf("%d, ",num_siguiente);
	
		contador++;
	}

	return 0;

}
