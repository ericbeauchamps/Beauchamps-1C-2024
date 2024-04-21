/*! @mainpage Guia 2 - Ejercicio 3
 *
 * @section genDesc General Description
 *
 * El programa permite, mediante el uso de un sensor de ultrasonido HC-SR04 y una pantalla LCD,
 * medir la distancia con el sensor y visualizarlo a través de la pantalla LCD, y también permite interaccion
 * con la PC a través del puerto serie.
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
 * | 13/04/2024 | Creacion del documento    |
 * | 19/04/2024 | Finalizacion del documento|
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def MEDICION_PERIODO
 *  @brief Valor de tiempo (en microsegundos) que debe pasar para poder realizar el sensado y visualizacion de la medicion
 */
#define MEDICION_PERIODO 1000000

/*==================[internal data definition]===============================*/
uint16_t distancia_medida, distancia_auxiliar = 0;
uint8_t constante_pulgadas = 0.39;
bool control_medicion = false;
bool control_hold = false;
bool control_unidad = true;

TaskHandle_t medir_distancia_handle = NULL; //Etiqueta
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

			if(distancia_auxiliar<=distancia_medida)
			{
				distancia_auxiliar = distancia_medida;
			}

			if (distancia_medida < 10)
			{
				//printf("Distancia medida menor a 10 cm\n");
				//printf("LED_1 OFF\n");
				LedOff(LED_1);
				//printf("LED_2 OFF\n");
				LedOff(LED_2);
				//printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else if (10 <= distancia_medida && distancia_medida < 20)
			{
				//printf("Distancia medida mayor a 10 cm pero menor a 20 cm\n");
				//printf("LED_1 ON\n");
				LedOn(LED_1);
				//printf("LED_2 OFF\n");
				LedOff(LED_2);
				//printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else if (20 <= distancia_medida && distancia_medida < 30)
			{
				//printf("Distancia medida mayor a 20 cm pero menor a 30 cm\n");
				//printf("LED_1 ON\n");
				LedOn(LED_1);
				//printf("LED_2 ON\n");
				LedOn(LED_2);
				//printf("LED_3 OFF\n");
				LedOff(LED_3);
			}

			else
			{
				//printf("Distancia medida mayor a 30 cm\n");
				//printf("LED_1 ON\n");
				LedOn(LED_1);
				//printf("LED_2 ON\n");
				LedOn(LED_2);
				//printf("LED_3 ON\n");
				LedOn(LED_3);
			}
		}

		else
		{
			//printf("Medicion apagada\n");
			//printf("LED_1 OFF\n");
			LedOff(LED_1);
			//printf("LED_2 OFF\n");
			LedOff(LED_2);
			//printf("LED_3 OFF\n");
			LedOff(LED_3);
		}

		// vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
	}
}

/**
 * @brief MostrarDistancia permite mostrar la distancia previamente sensada en el display LCD y en la PC a través del puerto serie.
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

			// UartSendString: manda un string al puerto que paso
			/* UartItoa: convierte un entero sin signo de 32 bits a  una 
			cadena de caracteres en una base especificada y devuelve un puntero a 
			la cadena resultante. La cadena resultante se almacena en un búfer 
			estático buf de tamaño 32.*/
			
			if(control_unidad)
			{
			UartSendString(UART_PC,"Distancia medida: ");
			UartSendString(UART_PC, (const char*)UartItoa(distancia_medida, 10));
			UartSendString(UART_PC," ");
			UartSendString(UART_PC,"cm");
			UartSendString(UART_PC,"\r\n"); //esto es el fin de linea abajo
			}

			else if(!control_unidad)
			{
			UartSendString(UART_PC,"Distancia medida: ");
			UartSendString(UART_PC, (const char*)UartItoa(distancia_medida*constante_pulgadas, 10));
			UartSendString(UART_PC," ");
			UartSendString(UART_PC,"pulgadas");
			UartSendString(UART_PC,"\r\n"); //esto es el fin de linea abajo
			}
		}

		else if (!control_medicion)
		{
			LcdItsE0803Off();
		}
		// vTaskDelay(MEDICION_PERIODO / portTICK_PERIOD_MS);
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
 * @brief FuncionTecla1 permite habilitar o deshabilitar la medición del sensor de distancia.
 */
void FuncionTecla1()
{
	control_medicion = !control_medicion;
}

/**
 * @brief FuncionTecla2 permite habilitar o deshabilitar el hold.
 */
void FuncionTecla2()
{
	control_hold = !control_hold;
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(medir_distancia_handle, pdFALSE); /* Envía una notificación a la tarea asociada*/
	vTaskNotifyGiveFromISR(mostrar_distancia_handle, pdFALSE);
}

/**
 * @brief ManejoPorTeclado permite la lectura de la teclada ingresa a través del teclado, y en función de dicha tecla realiza diversas funciones.
 */
void ManejoPorTeclado()
{
	uint8_t tecla_ingresada;
	
	UartReadByte(UART_PC, &tecla_ingresada);//va a leer la tecla que apreto en el teclado y me va a modificar mi variable

	if (tecla_ingresada == 'O')
	{
		FuncionTecla1();
	}

	else if (tecla_ingresada == 'H')
	{
		FuncionTecla2();
	}

	else if (tecla_ingresada == 'I')
	{
		control_unidad = !control_unidad;
	}

	else if (tecla_ingresada == 'M')
	{
		UartSendString(UART_PC,"Distancia maxima: ");
		UartSendString(UART_PC, (const char*)UartItoa(distancia_auxiliar, 10));
		UartSendString(UART_PC," ");
		UartSendString(UART_PC,"cm");
		UartSendString(UART_PC,"\r\n"); //esto es el fin de linea abajo
	}

	else
	{
		printf("Tecla ingresada incorrecta.\n");
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Actividad 3 - Proyecto: Medidor de distancia por ultrasonido c/interrupciones y puerto serie

	Modifique la actividad del punto 2 agregando ahora el puerto serie.
	Envíe los datos de las mediciones para poder observarlos en un terminal en la PC,
	siguiendo el siguiente formato:

	3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea “ \r\n”
	Además debe ser posible controlar la EDU-ESP de la siguiente manera:
	Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
	*/

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchesInit();

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200; // tasa de bits por segundo
	mi_serial.func_p = ManejoPorTeclado; //esta funcion va a hacer las interrupciones cuando haya apretado una tecla
	
	/*Configuro e inicializo la UART*/
	UartInit(&mi_serial);

	/*inicializo el timer*/
	timer_config_t timer_distancia = {
		.timer = TIMER_A,
		.period = MEDICION_PERIODO,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&timer_distancia);

	SwitchActivInt(SWITCH_1, &LeerSwitch, NULL); //interrupción
	SwitchActivInt(SWITCH_2, &LeerSwitch, NULL);

	xTaskCreate(&MedirDistancia, "Medir distancia", 2048, NULL, 5, &medir_distancia_handle); //tarea
	xTaskCreate(&MostrarDistancia, "Mostrar distancia", 2048, NULL, 5, &mostrar_distancia_handle);

	TimerStart(timer_distancia.timer);
}
/*==================[end of file]============================================*/