/*! @mainpage Guia 1 - Ejercicio 6
 *
 * @section genDesc General Description
 *
 * El programa permite mostrar datos a traves de una pantalla LCD y un buzer emite sonido cuando se elige una cantidad de digitos inadecuada
 *
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	 	    	| 	GPIO_9		|
 * | 		    	| 	GPIO_18		|
 * | 	 	    	| 	GPIO_19		|
 * | 	  LCD 	    | 	GPIO_20		|
 * | 	 	    	| 	GPIO_21		|
 * | 		    	| 	GPIO_22		|
 * | 		    	| 	GPIO_23		|
 * | 	 Buzer	    | 	GPIO_3		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 22/03/2024 | Se crea el documento	                         |
 * | 22/03/2024 | Se documenta	                         		 |
 * | 01/04/2024 | Se finaliza la documentacion	                 |
 * 
 * @author Eric Beauchamps (beauchampseric97@gmail.com)
 *
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
/** @def CONFIG_BLINK_PERIOD
 *  @brief Valor de tiempo que permanece encendido el buzer
*/
#define CONFIG_BLINK_PERIOD 500 
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/** 
 * @brief Codifica de decimal a BCD.
 * @param data Dato decimal de 32 bits.
 * @param digits Cantidad de digitos a codificar.
 * @param bcd_number Puntero de un vector que almacena los datos una vez que han sido codificados.
 */

void  ConvertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	uint8_t resto;
	for(uint8_t i = 0; i<digits; i++)
	{
		resto = data%10; //Mediante el operador % obtengo el dato del resto del cociente
        bcd_number[i] = resto; //Guardo dicho resto para obtener la codificacion de dicho digito decimal en BCD
		data /= 10; //Paso al siguiente digito
	}
}

/** 
 * @brief Permite manipular el estado de los pines para la codificacion.
 * @param BCD Dato decimal ya codificado que se desea mostrar.
 * @param Pines Puntero que almacena los pines del numero que se desea mostrar.
 */

void ManipularEstadoPinCodificacion(uint BCD, gpioConf_t *Pines)
{
	for(uint8_t i = 0; i<4;i++)
	{
		GPIOInit(Pines[i].pin, Pines[i].dir); //Inicializo los pines de codificacion
	}

	for(uint8_t i = 0; i<4;i++)	
	{
		if((BCD >> i) & 1) //(BCD & 1<<i) es equivalente en resultado
		{
			GPIOOn(Pines[i].pin);
		}
		else
		{
			GPIOOff(Pines[i].pin);
		}
	}
};

/**
 * @brief Permite visualizar en el display LCD los números elegidos por el usuario
 * @param data Dato decimal de 32 bits.
 * @param digits Cantidad de digitos a codificar.
 * @param bcd_number Puntero de un vector que almacena los datos una vez que han sido codificados.
 * @param PinesCodificacion Puntero de un vector que almacena los pines del numero que se desea mostrar.
 * @param PinesDigitos Puntero de un vector que almacena los pines del digito que se desea mostrar.
 * 
 */

void ManipularDisplayLCD(uint32_t data, uint8_t digits, gpioConf_t *PinesCodificacion, gpioConf_t *PinesDigitos, uint8_t * bcd_number)
{
	for(uint8_t i = 0; i<4;i++)
	{
		GPIOInit(PinesDigitos[i].pin, PinesDigitos[i].dir); //Inicializo los pines que controlan los digitos
	}

	ConvertToBcdArray(data, digits, bcd_number);

	for(uint8_t i = 0; i<3;i++)
	{
		GPIOOff(PinesDigitos[i].pin);
		ManipularEstadoPinCodificacion (bcd_number[i], PinesCodificacion);
		GPIOOn(PinesDigitos[i].pin);			
		GPIOOff(PinesDigitos[i].pin);
	}
};

/*==================[external functions definition]==========================*/
void app_main(void)
{
	gpioConf_t PinCodificacion[4];
	gpioConf_t PinDigito[3];
	gpioConf_t PinBuzer; //Defino pin para buzer (no es necesario para resolucion del ejercio)

	uint32_t Dato = 857; //Dato que quiero convertir 
	uint8_t CantidadDigitos = 3; //Cantidad de bits que se agrupan
	uint8_t NumeroBCD[CantidadDigitos]; //Almacenamiento de los numeros en BCD 

	//Configuro los pines de codificacion
	PinCodificacion[0].pin = GPIO_20;
	PinCodificacion[0].dir = GPIO_OUTPUT;
	PinCodificacion[1].pin = GPIO_21;
	PinCodificacion[1].dir = GPIO_OUTPUT;
	PinCodificacion[2].pin = GPIO_22;
	PinCodificacion[2].dir = GPIO_OUTPUT;
	PinCodificacion[3].pin = GPIO_23;
	PinCodificacion[3].dir = GPIO_OUTPUT;

	//Configuro los pines para controlar cada digito
	PinDigito[0].pin = GPIO_9;
	PinDigito[0].dir = GPIO_OUTPUT;
	PinDigito[1].pin = GPIO_18;
	PinDigito[1].dir = GPIO_OUTPUT;
	PinDigito[2].pin = GPIO_19;
	PinDigito[2].dir = GPIO_OUTPUT;

	//Configuro pin para buzer (no es necesario para resolucion del ejercio)
	PinBuzer.pin = GPIO_3;
	PinBuzer.dir = GPIO_OUTPUT;
	
	if(CantidadDigitos>=4) //Garantizo que la codificacion sea de manera adecuada ya que display LCD solo muestra hasta 3 digitos
	{
		GPIOInit(PinBuzer.pin, PinBuzer.dir);
		GPIOOn(PinBuzer.pin);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		GPIOOff(PinBuzer.pin);
	    printf ("La cantidad de digitos ingresada no es correcta.\n");
	}
	
	else
	{
	//Funcion para controlar los pines que permiten la visualizacion 
	ManipularDisplayLCD(Dato, CantidadDigitos, PinCodificacion, PinDigito, NumeroBCD);
	}
}

/*==================[end of file]============================================*/