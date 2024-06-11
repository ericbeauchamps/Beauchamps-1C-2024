/*! @mainpage Examen - 11/06/24 - Beauchamps Eric
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_SWITCH_PERIODO 1000 // Para evitar errores en la lectura de los switch: 1 mSeg
/*==================[internal data definition]===============================*/
bool irrigacion_habilitada = false;
/*==================[internal functions declaration]=========================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;


/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t teclas;
	SwitchesInit();

	/*Configuro e inicializo el GPIO para el sensor de humedad*/
	gpioConf_t PinSensorHumedad;
	PinSensorHumedad.pin = GPIO_19;
	PinSensorHumedad.dir = GPIO_INPUT;
	GPIOInit(PinSensorHumedad.pin, PinSensorHumedad.dir);

	/*Configuro e inicializo los GPIO  para las bombas*/
	gpioConf_t PinBombas[2]; // porque son 3 bombas
	PinBombas[0].pin = GPIO_20; //Esta es la bomba de agua
	PinBombas[0].dir = GPIO_OUTPUT;
	PinBombas[1].pin = GPIO_21;	//Esta es la bomba de pHA
	PinBombas[1].dir = GPIO_OUTPUT;
	PinBombas[2].pin = GPIO_22;	//Esta es la bomba de pHB
	PinBombas[2].dir = GPIO_OUTPUT;

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200; 
	mi_serial.func_p = NULL;	  
	UartInit(&mi_serial);

	while (1)
	{
		/*Hago la lectura de las teclas para habilitar/desabilitar el sistema*/
		teclas = SwitchesRead();
		switch (teclas) 
		{
		case SWITCH_1:
			irrigacion_habilitada = true;
			break;
		case SWITCH_2:
			irrigacion_habilitada = false;
			break;
		}

		vTaskDelay(CONFIG_SWITCH_PERIODO / portTICK_PERIOD_MS);
	}
}
/*==================[end of file]============================================*/