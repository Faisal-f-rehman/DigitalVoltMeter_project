#include "ADC.h"
#include "lcd.h"
#include "USART.h"


//---- ADC initialisation:
void init_ADC(void)
{
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOCEN;	//GPIOC clock enable
	ADC_input_port->MODER|=(3u<<(2*ADC_input_pin)| (0u<<(2*push_button_pin )));	//ADC input pin is analogue mode and pushbutton is input
	
	RCC->APB2ENR|=RCC_APB2ENR_ADC1EN;		//ADC clock enable
	ADC1->SQR1&=~ADC_SQR1_L;						//set number of conversions per sequence to 1
	ADC1->SQR3&=~ADC_SQR3_SQ1;					//clear channel select bits
	ADC1->SQR3|=ADC_Channel;						//set channel
	ADC1->CR2|=ADC_CR2_ADON;						//enable ADC
	
}


unsigned short read_adc(void)
{
	//ADC1->CR2|=ADC_CR2_SWSTART;				//start ADC conversion
	while((ADC1->SR&ADC_SR_EOC)==0);	//wait for ADC conversion complete EOC = end of conversion
	return ADC1->DR;									//return converted value
}

void push_button_hold(void)
{	unsigned short switch_bounce=0;
	unsigned short push_button=0x00000000;	
	
	push_button=MASK_BUTTON();						//check if pin 13 is high or low and assign to push_button	
	
	//switch bounce test
	if(push_button>0){switch_bounce++;}		//if button is pressed once enter and increment switch_bounce by 1
	if(switch_bounce == 1)								//when switch is pressed enter this statement 
	{	
		button_delay_us(50);								//delay to let switch bounce stabilise
		push_button=MASK_BUTTON();					//check if switch is still press
		if(push_button>0){switch_bounce++;}	//if switch is still pressed increment switch_bounce by 1
	}
	//if switch bounce test passed
	if(switch_bounce>=2)									//if switch_bounce passed the switch bounce test enter
	{	send_usart_2arg("1","m");						//bold/bright text
		send_usart_3arg("3","1","m");				//change text colour red 
		send_usart_curPos("11","69");				//move cursor position to row=11, column=69
		send_usart_2arg("2","K");						//erase the whole line
		send_usart_string("ON HOLD");				//write "ON HOLD" on terminal screen 
		//------------------
		switch_bounce=0;										//reset swicth_bounce back to zero
		cmdLCD_4bit(0x01); 									// clear LCD display
		cmdLCD_4bit(LCD_LINE1);							// select LCD display line 1
		string_putLCD("     ON HOLD");			// write ON HOLD in the middle of LCD display line 1
		cmdLCD_4bit(LCD_LINE2);							// select LCD display line 2
		string_putLCD("----------------");	// draw line on LCD display line 2
		button_delay_us(1000000);						// stay in loop for 10sec
		cmdLCD_4bit(0x01); 									// clear LCD display
		cmdLCD_4bit(LCD_LINE1);							// select LCD display line 1 
		//-----------------------
		send_usart_2arg("2","K");						//erase the whole line 
		send_usart_3arg("3","9","m");				//change text colour default
		send_usart_2arg("2","m");						//dim text
		send_usart_2arg("2","J");						//clear terminal screen
		usart_cur_home();										//send cursor to row=1 column = 1
	}
}

void button_delay_us(unsigned int us)		//argument is approximate number of micro-seconds to delay
{
	unsigned char i;
	while(us--)
	{
		for(i=0; i<SystemCoreClock/4000000; i++);
	}
}


//unsigned short read_adc(void)
//{
//	unsigned short ADC_conversion = 0;
//	
//	ADC1->CR2|=ADC_CR2_SWSTART;				//start ADC conversion
//	if((ADC1->SR&ADC_SR_EOC)==0){			//wait for ADC conversion complete EOC = end of conversion
//	ADC_conversion = ADC1->DR;									//return converted value
//	}
//	return ADC_conversion;
//}
