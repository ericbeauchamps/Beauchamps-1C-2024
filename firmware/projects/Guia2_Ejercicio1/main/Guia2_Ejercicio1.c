/*! @mainpage Guia 2 - Ejercicio 1
 *
 * @section genDesc General Description
 *
 * El programa permite, mediante el uso de un sensor de ultrasonido HC-SR04 y una pantalla LCD,
 * medir la distancia con el sensor y visualizarlo a través de la pantalla.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 			 	| 	GPIO_9		|
 * | 		 		| 	GPIO_18		|
 * | 		 		| 	GPIO_19		|
 * | 	 LCD 	    | 	GPIO_20		|
 * | 		 		| 	GPIO_21		|
 * | 		 		| 	GPIO_22		|
 * | 		 		| 	GPIO_23		|
 * | 		 		| 	GPIO_2		|
 * | 	HC-SR04	 	| 	GPIO_3		|
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Creacion del documento	                     |
 * | 08/04/2024 | Se finaliza el documento	                 	 |
 *
 * @author Eric Beauchamps (Beauchampseric97gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hc_sr04.h"
#include "led.h"
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lcditse0803.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/** @def SWITCH_PERIODO
 *  @brief Valor de tiempo que debe pasar para poder realizar una lectura del estado del switch
*/
#define SWITCH_PERIODO 200

/** @def MEDICION_PERIODO
 *  @brief Valor de tiempo que debe pasar para poder realizar el sensado y  visualizacion de la medicion
*/
#define MEDICION_PERIODO 1000

/*==================[internal data definition]===============================*/
uint16_t distancia_medida;
bool control_medicion = false;
bool control_hold = false;

/*==================[internal functions declaration]=========================*/

/** 
 * @brief MedirDistancia permite realizar el sensado de la distancia.
 * @param *pvParameter puntero que permite la configuración de la tarea.
 */
void MedirDistancia(void *pvParameter)
{
	while (1)
	{
		if (control_medicion)
		{
			distancia_medida = HcSr04ReadDistanceInCentimeters();

			if (distancia_medida < 10)
			{
				printf("Distancia medida menor a 10 cm\n");
				printf("LED_1 OFF\n");
				LedOff(LED_1);
				printf("LED_2 OFF\n");
				LedOff(LED_2);
				printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else if (10 <= distancia_medida && distancia_medida < 20)
			{
				printf("Distancia medida mayor a 10 cm pero menor a 20 cm\n");
				printf("LED_1 ON\n");
				LedOn(LED_1);
				printf("LED_2 OFF\n");
				LedOff(LED_2);
				printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else if (20 <= distancia_medida && distancia_medida < 30)
			{
				printf("Distancia medida mayor a 20 cm pero menor a 30 cm\n");
				printf("LED_1 ON\n");
				LedOn(LED_1);
				printf("LED_2 ON\n");
				LedOn(LED_2);
				printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else
			{
				printf("Distancia medida mayor a 30 cm\n");
				printf("LED_1 ON\n");
				LedOn(LED_1);
				printf("LED_2 ON\n");
				LedOn(LED_2);
				printf("LED_3 ON\n");
				LedOn(LED_3);
			}
		}
		else
		{
				printf("Medicion apagada\n");
				printf("LED_1 OFF\n");
				LedOff(LED_1);
				printf("LED_2 OFF\n");
				LedOff(LED_2);
				printf("LED_3 OFF\n");
				LedOff(LED_3);
		}

		vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
	}
}

/** 
 * @brief MostrarDistancia permite mostrar la distancia previamente sensada.
 * @param *pvParameter puntero que permite la configuración de la tarea.
 */

void MostrarDistancia(void *pvParameter)
{

	while (1)
	{
		if (control_medicion && !control_hold)
		{
			LcdItsE0803Write(distancia_medida);
		}

		else if (!control_medicion)
		{
			LcdItsE0803Off();
		}

		vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
	}
}

/** 
 * @brief LeerSwitches permite leer el estado de los switches.
 * @param *pvParameter puntero que permite la configuración de la tarea.
 */

void LeerSwitches(void *pvParameter)
{
	int teclas;
	while (1)
	{
		teclas = SwitchesRead();

		switch (teclas)
		{
		case SWITCH_1:

			control_medicion = !control_medicion;

			break;

		case SWITCH_2:

			control_hold = !control_hold;

			break;
		}
		vTaskDelay(SWITCH_PERIODO / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	/*Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con las siguientes funcionalidades:

	Mostrar distancia medida utilizando los leds de la siguiente manera:

	Si la distancia es menor a 10 cm, apagar todos los LEDs.
	Si la distancia está entre 10 y 20 cm, encender el LED_1.
	Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
	Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.

	Mostrar el valor de distancia en cm utilizando el display LCD.
	Usar TEC1 para activar y detener la medición.
	Usar TEC2 para mantener el resultado (“HOLD”).
	Refresco de medición: 1 s
	*/

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();

	xTaskCreate(&MedirDistancia, "Medir distancia", 2048, NULL, 5, NULL);
	xTaskCreate(&MostrarDistancia, "Mostrar distancia", 2048, NULL, 5, NULL);
	xTaskCreate(&LeerSwitches, "Leer los switches", 2048, NULL, 5, NULL);
}
/*==================[end of file]============================================*/