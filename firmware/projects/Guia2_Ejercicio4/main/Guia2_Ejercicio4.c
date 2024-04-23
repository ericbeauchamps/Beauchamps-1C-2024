/*! @mainpage Guia 2 - Ejercicio 4
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
 * | 19/04/2024 | Creación del documento		                 |
 *
 * @author Eric Beauchamps (beauchampseric97@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
/** @def PERIODO_MUESTREO
 *  @brief Valor de tiempo (en microsegundos) de muestreo
 */
#define PERIODO_MUESTREO 2000
/*==================[internal data definition]===============================*/
TaskHandle_t conversion_handle;

/*==================[internal functions declaration]=========================*/

void Conversion()
{
	uint16_t valor;

	AnalogStartContinuous(CH1); //Comienza la lectura analogica continua
	AnalogInputReadContinuous(CH1,&valor); //Leo el valor analogico
	UartSendString(UART_PC,(const char*) UartItoa(valor,10)); //Envio el dato convertido a ASCII
	UartSendString(UART_PC, "11,\r");
}

void FuncionTimer(void *param)
{
	vTaskNotifyGiveFromISR(conversion_handle, pdFALSE); /* Envía una notificación a la tarea asociada*/
}

void ManejoPorTeclado()
{
	uint8_t tecla_ingresada;
	
	UartReadByte(UART_PC, &tecla_ingresada);//va a leer la tecla que apreto en el teclado y me va a modificar mi variable

	if (tecla_ingresada == 'T')
	{
		
	}

	else if (tecla_ingresada == 'B')
	{
		
	}

	else
	{
		printf("Tecla ingresada incorrecta.\n");
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
SwitchesInit();

/*Configuro e inicializo el puerto serie*/
serial_config_t mi_serial;
mi_serial.port = UART_PC;
mi_serial.baud_rate = 115200; // tasa de bits por segundo
mi_serial.func_p = ManejoPorTeclado; //esta funcion va a hacer las interrupciones cuando haya apretado una tecla
	
/*Configuro e inicializo la UART*/
UartInit(&mi_serial);

/*Configuro el canal analogico*/
analog_input_config_t mi_analogico;
mi_analogico.input = CH1;
mi_analogico.mode = ADC_CONTINUOUS;
mi_analogico.func_p = Conversion;
mi_analogico.param_p = NULL;
mi_analogico.sample_frec = 20000;

/*Inicializo la lectura analogica*/
AnalogInputInit(&mi_analogico);

/*Configuro e inicializo el timer*/
timer_config_t timer_conversion = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO,
		.func_p = FuncionTimer,
		.param_p = NULL};
TimerInit(&timer_conversion);

}
/*==================[end of file]============================================*/