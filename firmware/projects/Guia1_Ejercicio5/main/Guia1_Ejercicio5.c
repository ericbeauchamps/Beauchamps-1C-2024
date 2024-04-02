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
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void ManipularEstadoPinCodificacion(uint BCD, gpioConf_t *Pines)
{
	for(uint8_t i = 0; i<4;i++)
	{
		GPIOInit(Pines[i].pin, Pines[i].dir); //Inicializo los pines
	}

	for(uint8_t i = 0; i<4;i++)	
	{
		if((BCD >> i) & 1)
		{
			GPIOOn(Pines[i].pin);
		}
		else
		{
			GPIOOff(Pines[i].pin);
		}
	}
};

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t NumeroBCD = 7;
	gpioConf_t Pin[4];

	//Configuro los pines
	Pin[0].pin = GPIO_20;
	Pin[0].dir = GPIO_OUTPUT;
	Pin[1].pin = GPIO_21;
	Pin[1].dir = GPIO_OUTPUT;
	Pin[2].pin = GPIO_22;
	Pin[2].dir = GPIO_OUTPUT;
	Pin[3].pin = GPIO_23;
	Pin[3].dir = GPIO_OUTPUT;

	ManipularEstadoPinCodificacion(NumeroBCD,Pin);
}
/*==================[end of file]============================================*/