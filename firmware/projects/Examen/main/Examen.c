/*! @mainpage Examen - 11/06/24 - Beauchamps Eric
 *
 * @section genDesc Se diseña un dispositivo basado en la ESP-EDU que permita controlar el riego y el pH de una plantera mediante el uso de dos sensores y tres bombas.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  		|   ESP32   	|
 * |:----------------------:|:--------------|
 * | 	sensor de humedad 	| 	GPIO_19		|
 * | 	sensor de pH	 	| 	GPIO_01		|
 * | 	Bomba de agua	 	| 	GPIO_20		|
 * | 	Bomba de phA	 	| 	GPIO_21		|
 * | 	Bomba de phB 		| 	GPIO_22		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | 				Description                      |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | 		Creacion del documento	                 |
 * | 11/06/2024 |		Modificacion del documento               |
 * | 11/06/2024 | 		Finalizacion del documento	             |
 *
 * @author Beauchamps Eric (beauchampseric97@gmail.com)
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
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
/** @def CONFIG_SWITCH_PERIODO
 *  @brief Valor de tiempo (en microsegundos) para evitar errores en la lectura de los switch,
 */
#define CONFIG_SWITCH_PERIODO 1000	 // Para evitar errores en la lectura de los switch: 1 mSeg
/** @def CONFIG_BOMBA_PERIODO
 *  @brief Valor de tiempo (en microsegundos) para que puedan regar las bombas,
 */
#define CONFIG_BOMBA_PERIODO 1000000 // Entiendo que debería haber un delay para evitar errores en las bombas: 1 Seg
/** @def PERIODO_MEDICION 
 *  @brief Valor de tiempo (en microsegundos) entre cada medicion
 */
#define PERIODO_MEDICION 3000000	 // Medicion: 3 Seg
/** @def PERIODO_ESTADO
 *  @brief Valor de tiempo (en microsegundos) para informar el estado del sistema
 */
#define PERIODO_ESTADO 5000000		 // Estado: 5 Seg
/** @def VALOR_BOMBA_BASICA
 *  @brief Valor de tensión (en mV) parz activar la bomba basica
 */
#define VALOR_BOMBA_BASICA 1284.7	 // Valor en mV, deducido por regla de 3
/** @def VALOR_BOMBA_ACIDA
 *  @brief Valor de tensión (en mV) parz activar la bomba acida
 */
#define VALOR_BOMBA_ACIDA 1435.7	 // Valor en mV, deducido por regla de 3
/** @def VALOR_BOMBA_ACIDA
 *  @brief Valor de tensión  de referencia (en mV)
 */
#define VOLTAJE_REFERENCIA 3000		// Voltaje de referencia en mV	
/*==================[internal data definition]===============================*/
bool irrigacion_habilitada = false, estado_humedad = false, estado_bomba_agua = false, estado_bomba_acida = false, estado_bomba_basica = false;
uint16_t valor_pH_milivolts = 0 , valor_pH_real = 0;
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

/**
 * @brief Permite la medicion de las variables de interes (Humedad y pH)
 */
void Medicion()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (irrigacion_habilitada == true) // Controlo si el sistema esta habilitado
		{
			GPIOOn(PinSensorHumedad.pin); // Enciendo el GPIO del sensor de humedad

			if (GPIORead(PinSensorHumedad.pin) == true) // Leo el estado actual del sensor y si es 1
			{
				estado_bomba_agua = true;							   // Bandera para saber el estado de la bomba de agua: false signficia apagada y true es prendida
				estado_humedad = false;								   // Bandera para saber el estado de humedad: false signficia incorrecta y true es correcta
				GPIOOn(PinBombas[0].pin);							   // Prendo la bomba de agua
				vTaskDelay(CONFIG_BOMBA_PERIODO / portTICK_PERIOD_MS); // delay para dar tiempo para regar durante 1 Seg
			}

			else
			{
				estado_bomba_agua = false; // Bandera para saber el estado de la bomba de agua: false signficia apagada y true es prendida
				estado_humedad = true;	   // Bandera para saber el estado de humedad: false signficia incorrecta y true es correcta
				GPIOOff(PinBombas[0].pin); // Apago la bomba de agua
			}

			AnalogInputReadSingle(CH1, &valor_pH_milivolts); // leo el valor de pH en milivolts

			/*Regla de 3 
	
			Determino cuando se debe prender la bomba basica
			3000 mV ------ 14 pH
			x = 1284.7 mV ------------- 6 pH

			Determino cuando se debe prender la bomba acida
			3000 mV ------ 14 pH
			x = 1435.7 mV ------------- 6,7 pH*/

			valor_pH_real = (valor_pH_milivolts*14)/VOLTAJE_REFERENCIA; //acá el pH real ya está listo por regla de 3

			if (valor_pH_milivolts < VALOR_BOMBA_BASICA)
			{

				GPIOOn(PinBombas[2].pin); // Prendo la bomba basica
				estado_bomba_basica = true;
				vTaskDelay(CONFIG_BOMBA_PERIODO / portTICK_PERIOD_MS); // delay para dar tiempo para regar durante 1 Seg
				GPIOOff(PinBombas[2].pin);							   // Apago la bomba basica
				estado_bomba_basica = false;
			}

			else if (valor_pH_milivolts > VALOR_BOMBA_ACIDA)
			{
				GPIOOn(PinBombas[1].pin); // Prendo la bomba acida
				estado_bomba_acida = true;
				vTaskDelay(CONFIG_BOMBA_PERIODO / portTICK_PERIOD_MS); // delay para dar tiempo para regar durante 1 Seg
				GPIOOff(PinBombas[1].pin);							   // Prendo la bomba acida
				estado_bomba_acida = false;
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

/**
 * @brief Informa acerca del estado del sistema (Valor de pH, estado de la humedad, bombas activadas/desactivadas)
 */
void Estado()
{
	while (1)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (irrigacion_habilitada == true)
		{
			UartSendString(UART_PC, "Valor del pH: ");
			UartSendString(UART_PC, (const char *)UartItoa(valor_pH_real, 10));
			UartSendString(UART_PC, "\n\r");

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

			if (estado_bomba_acida == true)
			{
				UartSendString(UART_PC, "Bomba acida prendida.\n\r");
			}

			else
			{
				UartSendString(UART_PC, "Bomba acida apagada.\n\r");
			}

			if (estado_bomba_basica == true)
			{
				UartSendString(UART_PC, "Bomba basica prendida.\n\r");
			}

			else
			{
				UartSendString(UART_PC, "Bomba basica apagada.\n\r");
			}
		}

		else
		{
			UartSendString(UART_PC, "Sistema deshabilitado.\n\r");
		}
	}
}
/**
 * @brief Timer para realizar la medicion en un tiempo determinado
 */
void TimerMedicion(void *param)
{
	vTaskNotifyGiveFromISR(medicion_handle, pdFALSE);
}
/**
 * @brief Timer para informar acerca del estado del sistema en un tiempo determinado
 */
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