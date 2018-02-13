#include <stm32f4xx.h>		//INCLUDE THE HEADER FILE FOR THIS MCU FAMILY
															//this file contains the definitions for register addresses and values etc...
#include "led.h"

//LED initialisation
void init_led(void)
{
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;		//GPIO B clock enable
	
	//CONFIGURE PORT PIN FUNCTIONS
	GPIOB->MODER&=~(3u<<(2*GREEN_LED) | (3u<<(2*BLUE_LED)) | (3u<<(2*RED_LED)));		//clear pin functions on GPIOB
	GPIOB->MODER|= (1u<<(2*GREEN_LED) | (1u<<(2*BLUE_LED)) | (1u<<(2*RED_LED)));		//set new pin functions on GPIOB
}

//--------------------green--------------------
//Turn green led on
void green_led_on(void)
{
	GPIOB->BSRR|=(1u<<GREEN_LED);
}
//Turn green led off
void green_led_off(void)
{
	GPIOB->BSRR|=(1u<<(16+GREEN_LED));
}


//-------------------blue----------------------
//Turn blue led on
void blue_led_on(void)
{
	GPIOB->BSRR|=(1u<<BLUE_LED);
}
//Turn blue led off
void blue_led_off(void)
{
	GPIOB->BSRR|=(1u<<(16+BLUE_LED));
}


//-----------------red-----------------
//Turn red led on
void red_led_on(void)
{
	GPIOB->BSRR|=(1u<<RED_LED);
}
//Turn red led off
void red_led_off(void)
{
	GPIOB->BSRR|=(1u<<(16+RED_LED));
}
