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
#define CONFIG_BOMBA_PERIODO 1000000 // Entiendo que debería haber un delay para evitar errores en las bombas: 1 Seg
#define PERIODO_MEDICION 3000000 // Medicion: 3 Seg
#define PERIODO_ESTADO 5000000	 // Estado: 5 Seg
/*==================[internal data definition]===============================*/
bool irrigacion_habilitada = false, estado_humedad = false, estado_bomba_agua = false;

TaskHandle_t medicion_handle = NULL;
TaskHandle_t estado_handle = NULL;

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t PinSensorHumedad;
gpioConf_t PinBombas[2]; // porque son 3 bombas
/*==================[internal functions declaration]=========================*/

void Medicion()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (irrigacion_habilitada == true) // Controlo si el sistema esta habilitado
		{
			GPIOOn(PinSensorHumedad.pin); // Enciendo el GPIO de lectura

			if (GPIORead(PinSensorHumedad.pin) == true) // Leo el estado actual del sensor y si es 1
			{
				estado_bomba_agua = true; // Bandera para saber el estado de la bomba de agua: false signficia apagada y true es prendida
				estado_humedad = false;	  // Bandera para saber el estado de humedad: false signficia incorrecta y true es correcta
				GPIOOn(PinBombas[0].pin); // Prendo la bomba de agua
				vTaskDelay(CONFIG_BOMBA_PERIODO / portTICK_PERIOD_MS); // delay para dar tiempo para regar durante 1 Seg
			}

			else
			{
				estado_bomba_agua = false; // Bandera para saber el estado de la bomba de agua: false signficia apagada y true es prendida
				estado_humedad = true;	   // Bandera para saber el estado de humedad: false signficia incorrecta y true es correcta
				GPIOOff(PinBombas[0].pin); // Apago la bomba de agua
			}
		}

		else
		{
			GPIOOff(PinSensorHumedad.pin); // Apago los GPIO
			GPIOOff(PinBombas[0].pin);	   // Apago los GPIO
			GPIOOff(PinBombas[1].pin);	   // Apago los GPIO
			GPIOOff(PinBombas[2].pin);	   // Apago los GPIO
		}
	}
}

void Estado()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (irrigacion_habilitada == true)
		{
			if (estado_humedad == true)
			{
				UartSendString(UART_PC, "Humedad correcta.\n\r");
			}

			else
			{
				UartSendString(UART_PC, "Humedad incorrecta.\n\r");
			}

			if (estado_bomba_agua == true)
			{
				UartSendString(UART_PC, "Bomba de agua prendida.\n\r");
			}

			else
			{
				UartSendString(UART_PC, "Bomba de agua apagada.\n\r");
			}
		}

		else
		{
			UartSendString(UART_PC, "Sistema deshabilitado.\n\r");
		}
	}
}

void TimerMedicion(void *param)
{
	vTaskNotifyGiveFromISR(medicion_handle, pdFALSE);
}

void TimerEstado(void *param)
{
	vTaskNotifyGiveFromISR(estado_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t teclas;
	SwitchesInit();

	/*Configuro e inicializo el GPIO para el sensor de humedad*/
	PinSensorHumedad.pin = GPIO_19;
	PinSensorHumedad.dir = GPIO_INPUT;
	GPIOInit(PinSensorHumedad.pin, PinSensorHumedad.dir);

	/*Configuro e  GPIO  para las bombas*/
	PinBombas[0].pin = GPIO_20; // Esta es la bomba de agua
	PinBombas[0].dir = GPIO_OUTPUT;
	PinBombas[1].pin = GPIO_21; // Esta es la bomba de pHA
	PinBombas[1].dir = GPIO_OUTPUT;
	PinBombas[2].pin = GPIO_22; // Esta es la bomba de pHB
	PinBombas[2].dir = GPIO_OUTPUT;
	GPIOInit(PinBombas[0].pin, PinBombas[0].dir);
	GPIOInit(PinBombas[1].pin, PinBombas[1].dir);
	GPIOInit(PinBombas[2].pin, PinBombas[2].dir);

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200;
	mi_serial.func_p = NULL;
	UartInit(&mi_serial);

	/*Configuro e inicializo el timer para la medicion*/
	timer_config_t timer_medicion = {
		.timer = TIMER_A,
		.period = PERIODO_MEDICION,
		.func_p = TimerMedicion,
		.param_p = NULL};
	TimerInit(&timer_medicion);

	xTaskCreate(&Medicion, "Medicion", 2048, NULL, 5, &medicion_handle);

	/*Configuro e inicializo el timer para informar el estado*/
	timer_config_t timer_estado = {
		.timer = TIMER_B,
		.period = PERIODO_ESTADO,
		.func_p = TimerEstado,
		.param_p = NULL};
	TimerInit(&timer_estado);

	xTaskCreate(&Estado, "Estado", 2048, NULL, 5, &estado_handle);

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

	TimerStart(TIMER_A);
	TimerStart(TIMER_B);
}
/*==================[end of file]============================================*/