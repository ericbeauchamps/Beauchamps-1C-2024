/*! @mainpage Template
 *
 * @section genDesc Electrogoniometro: se utiliza un potenciometro para medir el ángulo de la articulación del codo. 
 * El potenciometro cambia su resistencia en función del ángulo de rotación, lo que permite medir el ángulo de la 
 * articulación con precisión.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  		|   ESP32   	|
 * |:----------------------:|:--------------|
 * | 	Potenciometro	 	| 	GPIO_01		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    					    |
 * |:----------:|:----------------------------------------------------------------------|
 * | 13/05/2024 | Creacion del documento	                         			 		|
 * | 17/05/2024 | Implemente las tareas de conversion y visualizacion 		    		|
 * | 07/06/2024 | Calibre correctamente el goniometro y realice mejoras en el codigo 	|
 * | 15/06/2024 | Finalizacion del documento                     			 		    |
 * 
 *
 * @author Eric Beauchamps (beauchampseric97@gmail.com)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "ble_mcu.h"
#include "timer_mcu.h"
#include "delay_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD
 *  @brief Valor de tiempo (en milisegundos) para verificar el estado del BLE.
 */
#define CONFIG_BLINK_PERIOD 500

/** @def LED_BT
 *  @brief LED que permite visualizar el estado del BLE. LED apagado - BLE apagado, LED parpadeando - BLE desconectado, LED prendido - BLE conectado.
 */
#define LED_BT LED_1

/** @def PERIODO_MUESTREO_AD
 *  @brief Valor de tiempo (en microsegundos) de muestreo para la conversion de analogico a digital.
 */
#define PERIODO_MUESTREO_AD 20000

/** @def ANGULO_MINIMO
 *  @brief Angulo minimo admisible para la articulacion del codo: 0 °. Se utiliza para el mapeo.
 */
#define ANGULO_MINIMO 0

/** @def ANGULO_MAXIMO
 *  @brief Angulo maximo admisible para la articulacion del codo: 150 °. Se utiliza para el mapeo.
 */
#define ANGULO_MAXIMO 150

/** @def VALOR_MINIMO
 *  @brief Voltaje en milivolts correspondiente al ANGULO_MINIMO. Se utiliza para el mapeo.
 */
#define VALOR_MINIMO 735

/** @def VALOR_MAXIMO
 *  @brief Voltaje en milivolts correspondiente al ANGULO_MAXIMO. Se utiliza para el mapeo.
 */
#define VALOR_MAXIMO 2535
/*==================[internal data definition]===============================*/
volatile bool bt_conectado = false;

// 150 ° - 2535 mV
// 0 ° - 735 mV

uint16_t valor_medido, valor_minimo = 2535, valor_maximo = 0, angulo_minimo = 150, angulo_maximo = 0, angulo_medido;

TaskHandle_t conversion_AD_handle = NULL;
TaskHandle_t visualizacion_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función a ejecutarse ante un interrupción de recepción a través de la conexión BLE.
 * @param data      Puntero a array de datos recibidos
 * @param length    Longitud del array de datos recibidos
 */
void read_data(uint8_t *data, uint8_t length)
{
	if (data[0] == 'R')
	{
		// xTaskNotifyGive(fft_task_handle);
	}
}
/**
 * @brief Permite la conversión analogica - digital, y tambien obtiene el angulo medido despues de realizar un mapeo.
 */
void ConversionAD()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		AnalogInputReadSingle(CH1, &valor_medido);

		/*Evita errores a la hora de medir el angulo porque el potenciometro de desplazaba en mi brazo debido a la falta de una buena fijacion*/
		if (valor_medido < VALOR_MINIMO)
		{
			valor_medido = VALOR_MINIMO;
		}

		/*Evita errores a la hora de medir el angulo porque el potenciometro de desplazaba en mi brazo debido a la falta de una buena fijacion*/
		if (valor_medido > VALOR_MAXIMO)
		{
			valor_medido = VALOR_MAXIMO;
		}

		/*Realizo un mapeo para hallar el angulo medido*/
		angulo_medido = ANGULO_MINIMO + (valor_medido - VALOR_MINIMO) * (ANGULO_MAXIMO - ANGULO_MINIMO) / (VALOR_MAXIMO - VALOR_MINIMO);
	}
}

/**
 * @brief Permite hallar el angulo minimo y maximo, y tambien permite la visualizacion del angulo actual, el angulo mínimo y maximo, 
 * en el dispositivo movil.
 */
void Visualizacion()
{
	char msg[30];
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		/*Hallo el angulo maximo*/
		if (valor_maximo <= valor_medido)
		{
			valor_maximo = valor_medido;
			angulo_maximo = angulo_medido;
		}

		/*Hallo el angulo minimo*/
		if (valor_minimo >= valor_medido)
		{
			valor_minimo = valor_medido;
			angulo_minimo = angulo_medido;
		}

		/*Envio el angulo actual medido a traves del BLE al dispositivo movil*/
		sprintf(msg, "*D%u\n*", angulo_medido);
		BleSendString(msg);

		/*Envio el angulo minimo medido a traves del BLE al dispositivo movil*/
		sprintf(msg, "*M%u\n*", angulo_minimo);
		BleSendString(msg);

		/*Envio el angulo maximo medido a traves del BLE al dispositivo movil*/
		sprintf(msg, "*P%u\n*", angulo_maximo);
		BleSendString(msg);
	}
}

/**
 * @brief Funcion invocada en la interrupcion del timer A
 */
void TimerADC(void *param)
{
	if (bt_conectado == true)
	{
		vTaskNotifyGiveFromISR(conversion_AD_handle, pdFALSE);
		vTaskNotifyGiveFromISR(visualizacion_handle, pdFALSE); /* Envía una notificación a la tarea asociada. No siempre es necesaria*/
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Configuro e inicializo el BLE*/
	ble_config_t ble_configuration = {
		"Goniometro",
		read_data};
	BleInit(&ble_configuration);

	/*Inicializo los LEDS*/
	LedsInit();
	
	/*Configuro e inicializo el timer para la conversion analogica - digital y para la visualizacion de la informacion*/
	timer_config_t timer_conversion_AD = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO_AD,
		.func_p = TimerADC,
		.param_p = NULL};
	TimerInit(&timer_conversion_AD);
	TimerStart(TIMER_A);

	/*Tareas necesarias para la resolucion del proyecto*/
	xTaskCreate(&ConversionAD, "Conversor AD", 2048, NULL, 5, &conversion_AD_handle);
	xTaskCreate(&Visualizacion, "Visualizacion", 2048, NULL, 5, &visualizacion_handle);

	while (1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

		/*Manejo del bluetooth*/
		switch (BleStatus())
		{
		case BLE_OFF:
			LedOff(LED_BT);
			break;
		case BLE_DISCONNECTED:
			LedToggle(LED_BT);
			break;
		case BLE_CONNECTED:
			LedOn(LED_BT);

			/*Bandera booleana que permite realizar las tareas solo cuando el BLE de la placa esta conectado al BLE del dispositivo movil*/
			bt_conectado = true;

			break;
		}
	}
}
/*==================[end of file]============================================*/