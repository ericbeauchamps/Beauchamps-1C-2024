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
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "ble_mcu.h"
#include "timer_mcu.h"
#include "delay_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "ADXL335.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1

/** @def PERIODO_MUESTREO_AD
 *  @brief Valor de tiempo (en microsegundos) de muestreo para la conversion de analogico a digital.
 */
#define PERIODO_MUESTREO_AD 20000

#define ANGULO_MINIMO 0
#define ANGULO_MAXIMO 150
#define VALOR_MINIMO 735
#define VALOR_MAXIMO 2535
/*==================[internal data definition]===============================*/
volatile bool bt_conectado = false;

//

uint16_t valor_medido, valor_minimo = 2535, valor_maximo = 0, angulo_minimo = 150, angulo_maximo =0, angulo_medido;

TaskHandle_t conversion_AD_handle = NULL;
TaskHandle_t visualizacion_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función a ejecutarse ante un interrupción de recepción
 * a través de la conexión BLE.
 * 
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
// 150 ° - 2535
// 0 ° - 735
/**
 * @brief Permite la conversión analogica - digital
 */
void ConversionAD()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		AnalogInputReadSingle(CH1, &valor_medido);

		if(valor_medido < VALOR_MINIMO)
		{
			valor_medido = VALOR_MINIMO;
		}

		if(valor_medido > VALOR_MAXIMO)
		{
			valor_medido = VALOR_MAXIMO;
		}
		
		angulo_medido = ANGULO_MINIMO + (valor_medido - VALOR_MINIMO)*(ANGULO_MAXIMO - ANGULO_MINIMO) / (VALOR_MAXIMO - VALOR_MINIMO);
			
	}
}

void Visualizacion()
{
	char msg[30];
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (valor_maximo <= valor_medido)
		{
			valor_maximo = valor_medido;
			angulo_maximo = angulo_medido;

		}

		if (valor_minimo >= valor_medido)
		{
			valor_minimo = valor_medido;
			angulo_minimo = angulo_medido;
		}
		UartSendString(UART_PC, "Valor en bits: ");
		UartSendString(UART_PC, (const char *)UartItoa(valor_medido, 10)); // Envio el dato convertido a ASCII
		UartSendString(UART_PC, "\n\r");
		UartSendString(UART_PC, "Angulo: ");
		UartSendString(UART_PC, (const char *)UartItoa(angulo_medido, 10)); // Envio el dato convertido a ASCII
		UartSendString(UART_PC, "\n\r");

		sprintf(msg, "*D%u\n*", angulo_medido);
		BleSendString(msg);

		sprintf(msg, "*M%u\n*", angulo_minimo);
		BleSendString(msg);

		sprintf(msg, "*P%u\n*", angulo_maximo);
		BleSendString(msg);
	}
}
/**
 * @brief Función invocada en la interrupción del timer A
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
	/*Configuro el BLE*/
	ble_config_t ble_configuration = {
		"Goniometro",
		read_data};

	LedsInit();
	BleInit(&ble_configuration);
	/*Inicializo el acelerometro*/
	ADXL335Init();

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200; // tasa de bits por segundo
	mi_serial.func_p = NULL;	  // esta funcion va a hacer las interrupciones cuando haya apretado una tecla

	/*Configuro e inicializo la UART*/
	UartInit(&mi_serial);

	/*Configuro el canal analogico para el potenciometro
	analog_input_config_t mi_analogico;
	mi_analogico.input = CH0;
	mi_analogico.mode = ADC_SINGLE;
	mi_analogico.func_p = NULL;
	mi_analogico.param_p = NULL;
	AnalogInputInit(&mi_analogico);*/

	/*Configuro e inicializo el timer para la conversion analogica - digital*/
	timer_config_t timer_conversion_AD = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO_AD,
		.func_p = TimerADC,
		.param_p = NULL};
	TimerInit(&timer_conversion_AD);
	TimerStart(TIMER_A);

	xTaskCreate(&ConversionAD, "Conversor AD", 2048, NULL, 5, &conversion_AD_handle);
	xTaskCreate(&Visualizacion, "Visualizacion", 2048, NULL, 5, &visualizacion_handle);

	while (1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
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
			bt_conectado = true;
			break;
		}
	}
}
/*==================[end of file]============================================*/