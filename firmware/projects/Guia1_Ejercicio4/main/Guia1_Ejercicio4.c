/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void  ConvertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
		uint8_t resto;
		for(uint8_t i = 0; i<digits; i++)
		{
			resto = data%10; //Mediante el operador % obtengo el dato del resto del cociente
            bcd_number[i] = resto; //Guardo dicho resto para obtener la codificacion de dichi digito decimal en BCD
			data /= 10; //Paso al siguiente digito
		}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	 /*Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
	y un puntero a un arreglo donde se almacene los n dígitos. La función deberá convertir 
	el dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo pasado 
	como puntero.*/
	
    uint32_t Dato = 123; //Dato que quiero convertir - En BDC: 0010 0000 0010 0100 
	uint8_t CantidadDigitos = 3; //Cantidad de bits que se agrupan
	uint8_t NumeroBCD[CantidadDigitos]; //Almacenamiento de los numeros en BCD 

    if(CantidadDigitos>=10)
	{
	    printf ("La cantidad de digitos ingresada no es correcta.\n");
		//return -1; //Código de error convencial para cuando ocurre un error
	}
	
	else
	{
	    ConvertToBcdArray(Dato, CantidadDigitos, NumeroBCD); //Le paso a mi función

        for (uint8_t i = 0; i < CantidadDigitos; i++) 
        {   
            printf("BCD[%d]: %d\n", i, NumeroBCD[i]);
        }
	}
}
/*==================[end of file]============================================*/