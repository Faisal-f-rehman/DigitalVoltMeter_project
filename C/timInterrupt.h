#ifndef _TIMINTERRUPT_H
#define _TIMINTERRUPT_H

void init_TIM2 (void);
void init_TIM3 (void);
void init_TIM4 (void);

void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);

extern unsigned short lcd_time_scaler;
extern unsigned short usart_in;
extern unsigned short ADC_DC;

#endif
