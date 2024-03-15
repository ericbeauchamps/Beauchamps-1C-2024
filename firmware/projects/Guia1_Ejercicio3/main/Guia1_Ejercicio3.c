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
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 0
#define TOOGLE 45
/*==================[internal data definition]===============================*/
struct leds
{
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;    //indica el tiempo de cada ciclo
	uint8_t mode;       //ON, OFF, TOGGLE
} my_leds; 
/*==================[internal functions declaration]=========================*/
void ManejarLeds(struct leds *my_leds)
{
	switch (my_leds->mode="ON")
	{

	case ON:
		switch(my_leds->n_led)
		{
			case LED_1:
	    		LedOn(LED_1);
	   		 case LED_2:
				LedOn(LED_2);
	    	case LED_3:
	    		LedOn(LED_3);
	    break;
		}

	case OFF:
		switch(my_leds->n_led)
		{
			case LED_1:
	    		LedOff(LED_1);
	   		 case LED_2:
				LedOff(LED_2);
	    	case LED_3:
	    		LedOff(LED_3);
	    break;
		}	
	
	case TOOGLE:
		for(int j=0; j<my_leds->n_ciclos;j++)
		{
			switch(my_leds->n_led)
			{
				case LED_1:
	    			LedToggle(LED_1);
	   		 	case LED_2:
					LedToggle(LED_2);
	    		case LED_3:
	    			LedToggle(LED_3);
	    	break;
			}	
			
		}

		break;
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	//printf("Hello world!\n");

	struct leds *my_leds;

}
/*==================[end of file]============================================*/