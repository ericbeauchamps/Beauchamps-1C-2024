/*! @mainpage Guia 2 - Ejercicio 2
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
 * |   Date	    |       Description         |
 * |:----------:|:--------------------------|
 * | 12/04/2024 | Creacion del documento    |
 * | 12/04/2024 | Finalizacion del documento|
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
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def MEDICION_PERIODO
 *  @brief Valor de tiempo (en microsegundos) que debe pasar para poder realizar el sensado y visualizacion de la medicion
*/
#define MEDICION_PERIODO 1000000

/*==================[internal data definition]===============================*/
uint16_t distancia_medida;
bool control_medicion = false;
bool control_hold = false;

TaskHandle_t medir_distancia_handle = NULL;
TaskHandle_t mostrar_distancia_handle = NULL;

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
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */

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

		//vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
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
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (control_medicion && !control_hold)
		{
			LcdItsE0803Write(distancia_medida);
		}

		else if (!control_medicion)
		{
			LcdItsE0803Off();
		}

		//vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
	}
}

/** 
 * @brief LeerSwitch permite leer el estado de los switches.
 */
void LeerSwitch(void)
{
	int switch_actual;
	
	switch_actual = SwitchesRead();

    switch (switch_actual)
    {
        case SWITCH_1:
            control_medicion = !control_medicion;
            //printf("Switch 1 activado\n");
            break;
			
        case SWITCH_2:
            control_hold = !control_hold;
           // printf("Switch 2 activado\n");
            break;
    }
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
    vTaskNotifyGiveFromISR(medir_distancia_handle, pdFALSE);    /* Envía una notificación a la tarea asociada*/
	vTaskNotifyGiveFromISR(mostrar_distancia_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	/*Actividad 2 - Proyecto: Medidor de distancia por ultrasonido c/interrupciones
	Modifique la actividad del punto 1 de manera de utilizar interrupciones para el 
	control de las teclas y el control de tiempos (Timers). 
	*/

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();

	timer_config_t timer_distancia = {
        .timer = TIMER_A,
        .period = MEDICION_PERIODO,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_distancia);

	SwitchActivInt(SWITCH_1, &LeerSwitch, NULL);
	SwitchActivInt(SWITCH_2, &LeerSwitch, NULL);

	xTaskCreate(&MedirDistancia, "Medir distancia", 2048, NULL, 5, &medir_distancia_handle);
	xTaskCreate(&MostrarDistancia, "Mostrar distancia", 2048, NULL, 5,  &mostrar_distancia_handle);

	TimerStart(timer_distancia.timer);
}
/*==================[end of file]============================================*/