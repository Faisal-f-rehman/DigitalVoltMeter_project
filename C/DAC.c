#include "DAC.h"
#include <stm32f4xx.h>
#include "ADC.h"
#include "USART.h"
#include "timInterrupt.h"
#include "led.h"

//declare global variables used externally 
unsigned short wave_bank[32];
unsigned short sqrwave_bank_dac[2]; 
unsigned short sqrwave_bank_dac_freq[32];
unsigned short triWave_bank[32];

//---- DAC initialisation:
void init_DAC(void)
{
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN;			//DAC port clock enable
	DAC_port->MODER|=(3u<<(2*DAC_pin));			//DAC output pin set as anaglogue
	
	RCC->APB1ENR|=RCC_APB1ENR_DACEN;				//DAC clock enable
	DAC->CR|=DAC_CR_EN2;										//DAC 2 enabled
}

//sends data to dac
void output_dac(unsigned short d)
{
	DAC->DHR12R2=d;			//write data byte to DAC 2 output register, channel-1 12-bit right aligned data holding register
}
//--------------------------------------------------------------------------


//sends DC voltage to DAC
void DC_DAC(unsigned short adc_reading)
{
	if (ADC_DC == 1)						//enter when ADC_DC is set to 1. Set by timer 4 handler
	{
		green_led_on();						//turn green led on, for DC mode 
		blue_led_off();						//turn blue led off
		output_dac(adc_reading);	//send ADC data directly to DAC
		ADC_DC=0;									//set ADC_DC to 0, since ADC_DC is turned back to 1 by timer 4 handler, it is synced
	}
}
//--------------------------------------------------------------------------


//calculates sinewave and stores in wave_bank, does not sends it to DAC, sent to DAC by timer 4 handler
void sinewave_DAC(void)
{
	//declare local variables
	unsigned short wave; 					//holds converted sinewave value at run time 		
	short inc = 0;								//increment variable 0 to 32
	unsigned short div_2_adc = 0;	//holds adc data after its divided by 2
	
	//holds 32 values of a sinewave calculated manually on excel, formula: sin(x*2pi/32) where x is 1 to 32, scaled up by multiplying with 1000 to avoid floating points
	signed short sinewave_bank[32]={195, 383, 556, 707, 831, 924, 981, 1000,
																	981, 924, 831, 707, 556, 383, 195, 0,
																 -195,-383,-556,-707,-831,-924,-981,-1000,
																 -981,-924,-831,-707,-556,-383,-195, 0,};
	//run loop 32 times
	while(inc<=31)			
	{
		div_2_adc = read_adc()*0.5;																	//set sinewave point 0 by dividing ADC data into half 
		wave = ((div_2_adc*(sinewave_bank[inc]*0.001))+div_2_adc);	//multiply adc value by sinewave bank scaled back down by 1000 and shift wave up to keep the wave positive
		wave_bank[inc]=wave;																				//store all 32 converted values in wave_bank
		inc++;																											//increment inc by 1
	}
	inc=0;																												//reset inc back to 0
}
//--------------------------------------------------------------------------


//calculates triangular wave and stores in triWave_bank, does not sends it to DAC, sent to DAC by timer 4 handler
void triWave_DAC(void)			
{
	//declare local variable
	unsigned short triWave_ADC = 0;
	unsigned short buffer = 1;
	
	triWave_ADC = (read_adc()*0.03125);		//divide reading from ADC by 32, gives 17 steps up and 15 steps down 
	
	//formula = (ADC/32)*inc*2
	for(unsigned short inc=1; inc<=16; inc++)		//run loop 17 times
	{	
		triWave_bank[0] = triWave_ADC;							//manually set element 0 of triWave = ADC reading / 32
		triWave_bank[inc] = triWave_ADC*inc*2;		//store the rest of the 16 values as linear increment and multiply by 2 to scale it up
		
	}

	//formula = (ADC/32)*buffer*2
	for(unsigned short dec=30; dec>=17; dec--) 		//run loop 15 times
	{
			if(buffer>=15){buffer = 1;}						 		//RESET buffer to 1 if buffer = 15
			triWave_bank[31]=triWave_bank[0];					//manually set element 32 of triWave = ADC reading / 32
			triWave_bank[dec] = triWave_ADC*buffer*2;	//store the rest of the 14 values as linear increment, stored in triWave_bank in a reverse order and multiply by 2 to scale it up
			buffer++;																	//increment buffer by 1
	}
}
//--------------------------------------------------------------------------


//calculates square wave at same frequency of sine and tri wave and stores in sqrwave_bank_dac_freq, does not sends it to DAC, sent to DAC by timer 4 handler
void sqrWave_DAC_freq(void)
{	
	//stores 32 values first half = 0 and the second half = 1
	unsigned short squarewave_bank[32]={0,0,0,0,0,0,0,0,
																			0,0,0,0,0,0,0,0,
																			1,1,1,1,1,1,1,1,
																			1,1,1,1,1,1,1,1};
	//run loop 32 times
	for(unsigned short i=0;i<=31;i++)
	{
		//store all 32 converted values in squarewave_bank_dac_freq, formula:	ADC value*squarewave_bank, => first half give low and second half high
		sqrwave_bank_dac_freq[i]=squarewave_bank[i]*read_adc(); 
	}
}


void delay_us(unsigned int us)		//argument is approximate number of micro-seconds to delay
{
	unsigned char i;
	while(us--)
	{
		for(i=0; i<SystemCoreClock/4000000; i++);
	}
}
//--------------------------------------------------------------------------


//calculates square wave and stores in sqrwave_bank_dac, does not sends it to DAC, sent to DAC by timer 4 handler
//void sqrWave_DAC(void)
//{	
//	unsigned short squarewave_bank[2]={0,1};
//	for(unsigned short i=0;i<=1;i++)
//	{
//		sqrwave_bank_dac[i]=squarewave_bank[i]*read_adc();
//	}
//}


