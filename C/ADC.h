#ifndef _ADC_H
#define _ADC_H
#include <stm32f4xx.h>

#define ADC_input_port		GPIOC
#define ADC_input_pin			0
#define ADC_Channel				10
#define push_button_pin 	13

#define  MASK_BUTTON()			GPIOC->IDR&=(1u<<push_button_pin)  //AND IDR pin 13 by 1, if pin is high ie = 1 => MASK_BUTTON() = 1x1 = 1, otherwise MASK_BUTTON = 1x0 = 0

void init_ADC(void);
unsigned short read_adc(void);
void button_delay_us(unsigned int us);
#endif 
