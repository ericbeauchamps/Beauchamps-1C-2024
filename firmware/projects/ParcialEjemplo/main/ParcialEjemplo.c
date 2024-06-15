/*! @mainpage Odometro
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
 * |:----------:|:-----------------------------------------------| Peña dijo que acá hay que poner los perfiericos y pines usados, NO EN README
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
#include "Prueba.h"

/*==================[macros and definitions]=================================*/
#define RADIO 15 // No recuerdo bien el radio que dijo paco, yo supongo este

#define CONFIG_BLINK_PERIOD 1000 // Para evitar errores en la lectura de los switch 1 mSeg

#define PERIODO_MEDICION 20000 // Medicion cada 20 mSeg porque yo lo quiero

#define PERIODO_VISUALIZACION 1000000 // Visualizacion cada 1 Seg ---- No es necesario usar un timer para tiempos mayor a 500 mSeg
/*==================[internal data definition]===============================*/
float perimetro = 0, distancia_medida = 0;
bool medicion_habilitada = false, estado_actual = true, estado_previo = false;

TaskHandle_t visualizacion_handle = NULL;
TaskHandle_t medicion_handle = NULL;

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t PinSensor;

/*==================[internal functions declaration]=========================*/

void Medicion()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (medicion_habilitada == true)
		{
			GPIOOn(PinSensor.pin); // Enciendo el GPIO de lectura
			perimetro = 2 * M_PI * RADIO;

			/* 20 pulsos ------ perimetro (94,25 cm)
			 1 pulso ------ x = 4,71 cm*/

			/*Acá hay que programar un "detector de flancos" para que el medidor no siga aumentando
			la distancia sensada en caso de que nos frenamos mientras medimos*/

			estado_actual = GPIORead(PinSensor.pin); // Acá leo el estado actual de mi GPIO de entrada

			if (estado_actual == true && estado_previo == false)
			{
				distancia_medida = distancia_medida + 4.71; // la medición no va tener mucha exactitud pero se corrigen subiendo la cantidad de ranuras del disco
			}

			estado_previo = estado_actual;
		}

		else
		{
			GPIOOff(PinSensor.pin); // Apago el GPIO de lectura
			distancia_medida = 0; // hay que reiniciar la distancia medida cuando apago el medidor al presioanr el switch 2
		}
	}
}

void Visualizacion()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (medicion_habilitada == true)
		{
			UartSendString(UART_PC, "Distancia medida: ");
			UartSendString(UART_PC, (const char *)UartItoa(distancia_medida, 10)); // Envio el dato convertido a ASCII
			UartSendString(UART_PC, "cm\n\r");
		}
		else
		{
			UartSendString(UART_PC, "Lectura deshabilitada.\n\r");
		}

	}
}

void TimerVisualizacion(void *param)
{
	vTaskNotifyGiveFromISR(visualizacion_handle, pdFALSE); /* Envía una notificación a la tarea asociada. No siempre es necesaria*/
}

void TimerMedicion(void *param)
{
	vTaskNotifyGiveFromISR(visualizacion_handle, pdFALSE); /* Envía una notificación a la tarea asociada. No siempre es necesaria*/
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	uint8_t teclas;
	SwitchesInit();

	/*Configuro pin para el sensor e inicializo*/
	PinSensor.pin = GPIO_3;
	PinSensor.dir = GPIO_INPUT;
	GPIOInit(PinSensor.pin, PinSensor.dir);

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200; // tasa de bits por segundo
	mi_serial.func_p = NULL;	  // esta funcion va a hacer las interrupciones cuando haya apretado una tecla
	UartInit(&mi_serial);

	/*Configuro e inicializo el timer para informar la distancia medida ---- No es necesario usar un timer para tiempos mayor a 500 mSeg*/
	timer_config_t timer_visualizacion = {
		.timer = TIMER_A,
		.period = PERIODO_VISUALIZACION,
		.func_p = TimerVisualizacion,
		.param_p = NULL};
	TimerInit(&timer_visualizacion);

	/*Configuro e inicializo el timer para la medicion*/
	timer_config_t timer_medicion = {
		.timer = TIMER_A,
		.period = PERIODO_MEDICION,
		.func_p = TimerMedicion,
		.param_p = NULL};
	TimerInit(&timer_medicion);

	xTaskCreate(&Visualizacion, "Visualizacion", 2048, NULL, 5, &visualizacion_handle);
	xTaskCreate(&Medicion, "Medicion", 2048, NULL, 5, &medicion_handle);

	while (1)
	{
		teclas = SwitchesRead();
		switch (teclas) // Acá yo digo que los botones hacen esto porque paco dijo que el parcial estaba pensando para la EDU - CIA con 3 switchs
		{
		case SWITCH_1:
			medicion_habilitada = true;
			break;
		case SWITCH_2:
			medicion_habilitada = false;
			break;
		}

		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}

	TimerStart(TIMER_A);
	TimerStart(TIMER_B);
}
/*==================[end of file]============================================*/