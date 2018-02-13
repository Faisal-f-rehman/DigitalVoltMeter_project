/*FAISAL FAZAL-UR-REHMAN 10538828
	ELEC240 Coursework*/

#include <stm32f4xx.h>
#include "PLL_Config.c"
#include "ADC.h"
#include "DAC.h"
#include "lcd.h"
#include "timInterrupt.h"
#include "led.h"
#include "USART.h"
#include "spi.h"


#define lcd_speed_div	10000			//set lcd display speed to make it easier to read, speed(Hz)= 48000/lcd_speed_div

//PORTS:
//DAC GPIOAEN PIN  5
//SPI GPIOAEN PIN 4 -&- GPIOBEN PINS 3, 4, 5
//LED GPIOBEN PINS 0, 7 & 14
//ADC GPIOCEN PINS 0 & 10
//LCD GPIODEN PINS 0 to 7, 11, 12, 13
//USART GPIODEN PINS 8 & 9


//-----Global Variables-----//
unsigned short ADC_DATA;		//used to hold data from the adc



int main(void)
{
	//----------------Initialisation and setup----------------//
	//PLL_Config();
	SystemCoreClockUpdate();
	
	//Initiate Peripherals
	initLCD();		//initiate lcd
	init_USART();	//initiate usart
	init_ADC();		//initiate ADC
	init_DAC();		//initiate DAC
	init_TIM3();	//configure and initiate Timer 3
	init_TIM4();	//configure and initiate Timer 4
	init_led();		//initiate Leds
	init_spi();		//initiate SPI
	
		
	while(1)
	{
		
		//----ADC----
		ADC_DATA=read_adc();			//assign ADC value to ADC_DATA
		
		
		//----SPI----
		transfer_spi(ADC_DATA/16.05);	//sends data to slave
		
		
		//----DAC----
		//waves enabled by switch statement in timer 4 handler, only 1 enabled at a time
		DC_DAC(ADC_DATA);		//sends DC voltage to DAC
		sinewave_DAC();			//calculates and stores sinewave, to be sent to DAC by timer 4 handler
		sqrWave_DAC_freq();	//calculates and stores squarewave, to be sent to DAC by timer 4 handler
		triWave_DAC();			//calculates and stores triangularwave, to be sent to DAC by timer 4 handler
		
		
		//----Hold Button----
		push_button_hold(); //checks if onboard blue button (B1) is pressed if pressed it puts every thing on hold for 10sec (function in ADC library)
		
		
		//----LCD----
		if (lcd_time_scaler>lcd_speed_div)			//lcd_time_scaler is incremented by timer 3 on each clock cycle and lcd_speed_div is #define ie constant value,//
		{																				//				this is not a necessary step but since it slows down lcd display its easier to read values					 //
		int_to_lcd(ADC_DATA);				//writes voltage on lcd screen
		bar_to_LCD(ADC_DATA);				//writes bar representation on lcd screen
		lcd_time_scaler=0;					//reset variable lcd_time_scaler, incremented in timer 3
		}
		
	
		//----USART----
		usart_frame();							//sends pre-defined frame on top of terminal
		usart_mode_sel();						//only writes mode types on terminal screen, it does not read from the terminal
		ADC_volt_to_usart(ADC_DATA);//writes voltage on terminal
		receive_usart();						//reads from terminal and writes back to terminal what was recived to pre-defined position
		ADC_bar_USART();						//writes bar representation on terminal
		usart_switch_wave();				//switches outputs to led, DAC and USART between DC, sinewave, squarewave and triangularwave 
		usart_cur_home();						//RESET cursor position on terminal		
	}
}
