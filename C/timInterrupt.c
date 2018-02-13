#include "timInterrupt.h"
#include "stm32f4xx.h"                  // Device header
#include "ADC.h"
#include "DAC.h"
#include "led.h"
#include "USART.h"

//Global variables used externally
unsigned short mode_select = 2;
unsigned short led_time_scaler = 4800; //Variable to divide sampling rate, 48000/4800=10Hz
unsigned short sqr_i = 0;
unsigned short tri_i = 0;
unsigned short lcd_time_scaler = 0;
unsigned short usart_in = 0;
unsigned short ADC_DC = 0;
short sine = -1;
short tri = -1;



//--------------Timer 2
//void init_TIM2 (void)
//{
//	RCC->APB1ENR|=RCC_APB1ENR_TIM2EN;			// enable Timer2 clock
//	TIM2->DIER|=TIM_DIER_UIE;							//timer uptdate interrupt enabled. APB clock is Fcy/2 = 180MHz/2 = 90MHz
//	TIM2->PSC=26-1;												//divide APB clock by 256 = 90MHz/256 = 351kHz
//	TIM2->ARR=12;													//counter reload value, gives a timer period of 10ms when F_APB = 90MHz and PSC = 256
//	TIM2->CNT=0;													//CNT register is set to zero once the APB clock has reached value = ARR
//	NVIC->ISER[0]|=(1u<<28);							//timer 2 global interrupt enabled
//	TIM2->CR1|=TIM_CR1_CEN;								//start timer counter		
//}

//--------------Timer 3
void init_TIM3(void)
{
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;		//timer 3 clock enabled
	TIM3->DIER|=TIM_DIER_UIE;						//timer uptdate interrupt enabled
																			//APB clock is HSI = 16MHz
	TIM3->PSC=0;												//divide APB clock by 1 = 16MHz/1 = 16MHz ---> then divided by ARR:
	TIM3->ARR=333;											//counter reload value, gives a timer period of 20.8us, Freq = 16MHz/333 = 48kHz
	TIM3->CNT=0;												//zero timer counter
	NVIC->ISER[0]|=(1u<<29);						//timer 3 global interrupt enabled
	TIM3->CR1|=TIM_CR1_CEN;							//start timer counter
}

//--------------Timer 4
void init_TIM4(void)
{	
		RCC->APB1ENR|=RCC_APB1ENR_TIM4EN;		//timer 4 clock enabled
		TIM4->DIER|=TIM_DIER_UIE;						//timer uptdate interrupt enabled
																				//APB clock is HSI = 16MHz
		TIM4->PSC=0;												//divide APB clock by 1 = 16MHz/1 = 16MHz ---> then divided by ARR:
		TIM4->ARR=333;											//counter reload value, gives a timer period of 20.8us, Freq = 16MHz/333 = 48kHz
		TIM4->CNT=0;												//zero timer counter
		NVIC->ISER[0]|=(1u<<30);						//timer 4 global interrupt enabled
		TIM4->CR1|=TIM_CR1_CEN;							//start timer counter
}	

//------------ISR 2:
//void TIM2_IRQHandler(void)			//TIMER 2 INTERRUPT SERVICE ROUTINE
//{ 
//	
//	TIM2->SR&=~TIM_SR_UIF;		 																												//clear interrupt flag in status register
//}


//--------------ISR 3:
void TIM3_IRQHandler(void)				//TIMER 3 INTERRUPT SERVICE ROUTINE
{	
	
	//----LCD Time Scaler
	lcd_time_scaler++;						//this variable is incremented by 1 on each timer clock cycle and is used to slow down lcd display 
	//---------------------------------------------------
	
	//----Sample rate LED
	if (led_time_scaler>=2400)		//keep red led off for 2400 cycles
	{	
		red_led_off();							//turn red led off
		led_time_scaler--;					//reduce time scaler count by 1 per cycle
	}
	if (led_time_scaler<2400)			//keep red led on for 2400 cycles
	{
		red_led_on();								//turn red led on
		led_time_scaler--;					//reduce time scaler count by 1 per cycle
		if(led_time_scaler<1)				//RESET time scaler after 4800 cycles
		{
			led_time_scaler=4800;			//RESET time scaler back to 4800
		}
	}
	//---------------------------------------------------

	//----ADC
	ADC1->CR2|=ADC_CR2_SWSTART;			//start ADC conversion
	//---------------------------------------------------
	
	//---- clear timer flag
	TIM3->SR&=~TIM_SR_UIF;					//clear ISR flag
}


//----------------------------------------------------------------------//


//--------------ISR 4:
void TIM4_IRQHandler(void)			//TIMER 4 INTERRUPT SERVICE ROUTINE
{	
	//----DAC-controlled by terminal----//
	if((USART_MODULE->SR&USART_SR_RXNE) && (usart_in<1)) //enter if char has been received
	{
		usart_in++; 		//this variable triggers function, received_usart in usart.c which writes reads from terminal assigns it to read_usart(see below) and writes back to terminal
	}

	switch (read_usart) 									//variable read_usart holds value written to terminal in usart.c and triggered above 
	{
		case '1':
			ADC_DC=1;														//triggers function DC_ADC() in DAC.c for DC representation											
			break;
		case '2':
			blue_led_on();											//turn blue led on for AC mode
			green_led_off();										//turn green led off
			if (sine == 32){sine = -1;}					//if sinewave sample 32 has been sent go back to sample 0 
			output_dac(wave_bank[sine++]);			//send each of 32 samples of sinewave to DAC starting from sample 0
			break;
		case '3':
			blue_led_on();														//turn blue led on for AC mode
			green_led_off();													//turn green led off 
			output_dac(sqrwave_bank_dac_freq[sqr_i++]);	//send each of 32 samples of squarewave to DAC starting from sample 0
			if (sqr_i >= 32){sqr_i=0;}								//if squarewave sample 32 has been sent go back to sample 0 
			break;
		case '4':
			blue_led_on();														//turn blue led on for AC mode
			green_led_off();													//turn green led off
			output_dac(triWave_bank[tri_i++]);				//send each of 32 samples of triangularwave to DAC starting from sample 0
			if (tri_i >= 32){tri_i=0;}								//if triangularwave sample 32 has been sent go back to sample 0
			break;
	}	
	
	TIM4->SR&=~TIM_SR_UIF;							//clear interrupt flag in status register
}
