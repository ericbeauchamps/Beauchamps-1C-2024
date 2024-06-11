/*! @mainpage Goniometro
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
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_MUESTREO_AD 20000 // Yo lo decidi

#define VOLTAJE_REFERENCIA 3.3
/*==================[internal data definition]===============================*/
uint16_t voltaje_medido = 0;
float angulo_medido = 0;
bool habilitar_medicion = true; // Acá hay que usar una bandera booleala para habilitar/deshabilitar la medicion con los switch. No lo termine


TaskHandle_t conversion_AD_handle = NULL;
TaskHandle_t visualizacion_handle = NULL;
/*==================[internal functions declaration]=========================*/
void ConversionAD()
{
	
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		//Acá falta un if con la bandera "habilitar_medicion". Creo que la consigna tambien pedia hallar el mayor angulo sensado 
		AnalogInputReadSingle(CH1, &voltaje_medido); // lee el valor ingresado en milivolts

		//Paco dijo que a 0° le corresponden 0 V y para 180° le corresponden 3.3 V

		angulo_medido = (voltaje_medido*180)/VOLTAJE_REFERENCIA; //acá el ángulo ya está listo por regla de 3

	}
}

void Visualizacion()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		UartSendString(UART_PC, "Angulo: ");
		UartSendString(UART_PC, (const char *)UartItoa(angulo_medido, 10)); // Envio el dato convertido a ASCII
		UartSendString(UART_PC, "\n\r");
	}
}
/**
 * @brief Función invocada en la interrupción del timer A
 */
void TimerADC(void *param)
{
	vTaskNotifyGiveFromISR(conversion_AD_handle, pdFALSE);
	vTaskNotifyGiveFromISR(visualizacion_handle, pdFALSE); /* Envía una notificación a la tarea asociada. No siempre es necesaria*/
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t teclas;
	SwitchesInit();

	/*Configuro e inicializo el puerto serie*/
	serial_config_t mi_serial;
	mi_serial.port = UART_PC;
	mi_serial.baud_rate = 115200; // tasa de bits por segundo
	mi_serial.func_p = NULL;	  // esta funcion va a hacer las interrupciones cuando haya apretado una tecla
	UartInit(&mi_serial);

	/*Configuro e inicializo el timer para la conversion analogica - digital*/
	timer_config_t timer_conversion_AD = {
		.timer = TIMER_A,
		.period = PERIODO_MUESTREO_AD,
		.func_p = TimerADC,
		.param_p = NULL};
	TimerInit(&timer_conversion_AD);

	xTaskCreate(&ConversionAD, "Conversor AD", 2048, NULL, 5, &conversion_AD_handle);
	xTaskCreate(&Visualizacion, "Visualizacion", 2048, NULL, 5, &visualizacion_handle);

	//Acá falta el código para leer los switch y modificar la bandera "habilitar_medicion"
	
	TimerStart(TIMER_A);
}
/*==================[end of file]============================================*/