#ifndef _USART_H_
#define _USART_H_
#include <stm32f4xx.h>

#define USART_MODULE	USART3
#define USART_PORT		GPIOD
#define USART_TX_pin	8
#define USART_RX_pin	9
#define BAUDRATE			115200
#define bar_resolution 58

void send_usart(unsigned char d);
void init_USART(void);
void send_usart_string(char input[]);
void send_usart_2arg(char num[], char cmd[]);
void volt_to_USART(unsigned short ADC_readings);
void usart_cur_home(void);
void send_usart_curPos(char row[], char col[]);
void send_usart_curPos_int(unsigned short row, unsigned short col);
void send_usart_3arg(char ascii1[], char ascii2[], char ascii3[]);
void usart_frame(void);
void usart_mode_sel(void);
void ADC_volt_to_usart(unsigned short int_to_ascii);
void receive_usart(void);
void ADC_bar_USART(void);
void sine_to_usart(void);
void int_to_USART(unsigned short int_value);
void usart_delay_us(unsigned int us);
void usart_switch_wave(void);
void square_to_usart(void);
void tri_to_usart(void);
void erase_state_machine(unsigned short x[]);
	
extern char read_usart;
#endif
