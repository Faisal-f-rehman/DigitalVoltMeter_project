#ifndef _DAC_H
#define _DAC_H
#include <stm32f4xx.h>

#define DAC_port	GPIOA
#define DAC_pin		5

extern unsigned short wave;
void init_DAC(void);
void output_dac(unsigned short d);
void sinewave_DAC(void);
void triWave_DAC(void);
void delay_us(unsigned int us);
void sqrWave_DAC(void);
void push_button_hold(void);
void sqrWave_DAC_freq(void);
void DC_DAC(unsigned short adc_reading);

extern unsigned short wave_bank[32];
extern unsigned short triWave_bank[32];
extern unsigned short sqrwave_bank_dac[2];
extern unsigned short sqrwave_bank_dac_freq[32];
#endif
