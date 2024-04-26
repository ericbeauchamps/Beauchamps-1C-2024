/*! @mainpage Guia 2 - Ejercicio 4
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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
/** @def PERIODO_MUESTREO_ADC
 *  @brief Valor de tiempo (en microsegundos) de muestreo
 */
#define PERIODO_MUESTREO_ADC 2000

#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
TaskHandle_t conversion_ADC_handle;
TaskHandle_t conversion_DAC_handle;

TaskHandle_t main_task_handle = NULL;

const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/

void ConversionDAC()
{
	AnalogOutputInit();

	for (int i = 0; i < BUFFER_SIZE; i++) 
	{
        AnalogOutputWrite(ecg[i]);
    }

}

void TimerDAC(void *param)
{
	
}

void ConversionADC()
{
	uint16_t valor;

	AnalogInputReadSingle(CH1, &valor);
	/*AnalogStartContinuous(CH1); //Comienza la lectura analogica single
	AnalogInputReadContinuous(CH1,&valor); //Leo el valor analogico single*/
	UartSendString(UART_PC,(const char*) UartItoa(valor,10)); //Envio el dato convertido a ASCII
	UartSendString(UART_PC, "11,\r");
}

void TimerADC(void *param)
{
	vTaskNotifyGiveFromISR(conversion_ADC_handle, pdFALSE); /* Envía una notificación a la tarea asociada*/
}

void LeerSwitch(void)
{
	int switch_actual;

	switch_actual = SwitchesRead();

	switch (switch_actual)
	{
		case SWITCH_1:
			
			break;

		case SWITCH_2:
			
			break;
	}
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

	else if (tecla_ingresada == 'R')
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
mi_analogico.mode = ADC_SINGLE;
mi_analogico.func_p = ConversionADC;
mi_analogico.param_p = NULL;
mi_analogico.sample_frec = 20000;

/*Inicializo la lectura analogica*/
AnalogInputInit(&mi_analogico);

SwitchActivInt(SWITCH_1, &LeerSwitch, NULL); //interrupción
SwitchActivInt(SWITCH_2, &LeerSwitch, NULL);

/*Configuro e inicializo el timer para la conversion analogica a digital*/
timer_config_t timer_conversion_ADC = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO_ADC,
		.func_p = TimerADC,
		.param_p = NULL};
TimerInit(&timer_conversion_ADC);

}
/*==================[end of file]============================================*/