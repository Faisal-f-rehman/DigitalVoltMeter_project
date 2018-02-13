#include "lcd.h"
#include "timInterrupt.h"


//-------------------------------------LCD initialisation:

void initLCD(void)
{
	//SystemCoreClockUpdate();
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIODEN;	//enable LCD port clock

	
	//CONFIG LCD GPIO PINS
	LCD_PORT->MODER&=~(					//clear pin direction settings
		(3u<<(2*LCD_RS_pin))
		|(3u<<(2*LCD_RW_pin))
		|(3u<<(2*LCD_E_pin))
		|(0xffff<<(2*LCD_D0_pin))
		);

	LCD_PORT->MODER|=(				//reset pin direction settings to digital outputs
		(1u<<(2*LCD_RS_pin))
		|(1u<<(2*LCD_RW_pin))
		|(1u<<(2*LCD_E_pin))
		|(0x5500<<(2*LCD_D0_pin))
		);

	//LCD INIT COMMANDS
	clr_LCD_RS();					//all lines default low
	clr_LCD_RW();
	clr_LCD_E();
	
	lcd_delayus(25000);		//25ms startup delay
	
	set_LCD_data(0x30);	//Function set: 1 Line, 8-bit, 5x8 dots 3ms
	LCD_strobe();
	lcd_delayus(50);		//wait 50us
	
	set_LCD_data(0x20);	//Function set: 1 Line, 4-bit, 5x8 dots
	LCD_strobe();
	lcd_delayus(50);	//wait 39us
	LCD_strobe();
	lcd_delayus(50);	//wait 39us
	
	
	cmdLCD_4bit(0x28);	//Function set: 2 Line, 4-bit, 5x8 dots
	cmdLCD_4bit(0x0c);	//Display on, Cursor blinking command
	cmdLCD_4bit(0x01);	//Clear LCD	
	cmdLCD_4bit(0x06);	//Entry mode, auto increment with no shift
	
	cmdLCD_4bit(LCD_LINE1);	//Select LCD display line 1
}

//---------------------Delay:
void lcd_delayus(unsigned int us)		//blocking delay for LCD, argument is approximate number of micro-seconds to delay
{
	unsigned char i;
	while(us--)
	{
		for(i=0; i<SystemCoreClock/4000000; i++);
	}
}

//--------------------write to LCD (8-bit):
void set_LCD_data(unsigned char d)
{
	LCD_PORT->BSRR=(0xff<<(LCD_D0_pin+16));	//clear data lines
	LCD_PORT->BSRR=(d<<LCD_D0_pin);					//put data on lines
}

//--------------write least significant nibble to LCD (4-bit)
void set_LCD_data_4bitLSB(unsigned char d)
{	d&=0x0f;
	LCD_PORT->BSRR=(0xff<<(LCD_D0_pin+16));	//clear data lines
	LCD_PORT->BSRR=(d<<LCD_D4_pin);					//put data on lines
}
//--------------write most significant nibble to LCD (4-bit)
void set_LCD_data_4bitMSB(unsigned char d)
{	d&=0xf0;
	LCD_PORT->BSRR=(0xff<<(LCD_D0_pin+16));	//clear data lines
	LCD_PORT->BSRR=(d<<LCD_D0_pin);					//put data on lines
}

//---------------------Enable pin handler:
void LCD_strobe(void)		//10us high pulse on LCD enable line
{
	lcd_delayus(10);			// delay 10us
	set_LCD_E();					// set enable pin high
	lcd_delayus(10);			// delay 10us
	clr_LCD_E();					// set enable pin low
}

//-----------------send 2 seperate nibbles to LCD control register
void cmdLCD_4bit(unsigned char cmd)		
{
	lcd_delayus(2000);						//wait 3ms to make sure lcd is free
	clr_LCD_RS();									//control command
	clr_LCD_RW();									//write command
	set_LCD_data_4bitMSB(cmd);		//set data on bus
	LCD_strobe();
	set_LCD_data_4bitLSB(cmd);
	LCD_strobe();									//apply command
}

//-----------------send 2 seperate nibbles to LCD control register
void cmdLCD_4bit_mask(unsigned char cmd)		
{
	loading();
	//lcd_delayus(1000);						//wait 3ms to make sure lcd is free
	clr_LCD_RS();									//control command
	clr_LCD_RW();									//write command
	set_LCD_data_4bitMSB(cmd);		//set data on bus
	LCD_strobe();
	set_LCD_data_4bitLSB(cmd);
	LCD_strobe();									//apply command
}

//----------------sends a char to the LCD display (8-bit)
void putLCD(unsigned char put)	
{
	loading();
	//WaitLcdBusy();				//wait for LCD to be not busy
	set_LCD_RS();					//text command
	clr_LCD_RW();					//write command
	set_LCD_data(put);		//set data on bus
	LCD_strobe();					//apply command
}

//----------------sends a char to the LCD display (4-bit)
void putLCD_4bit(unsigned char put)	//sends a char to the LCD display
{
	loading();									//busy flag check
	set_LCD_RS();								//text command
	clr_LCD_RW();								//write command
	set_LCD_data_4bitMSB(put);	//send MSB nibble to LCD
	LCD_strobe();								//strobe enable line to apply command
	set_LCD_data_4bitLSB(put);	//send LSB nibble to LCD
	LCD_strobe();								//strobe enable line to apply command
}

//-----------------------Busy Flag checker:
void loading(void)
{
	LCD_PORT->MODER&=~(3u<<(2*FLAG_pin));
	LCD_PORT->MODER|=(0u<<2*FLAG_pin);			//Set pin 7 to input
						
	SET_FLAG_pin();													//set RS=0, R/W=1 and E=1
	//MASK_FLAG();														//mask pin 7 by ANDing current value with 1 on pin 7
	
	unsigned int flag_status=0x00000000;
	flag_status=MASK_FLAG();											//LCD_PORT->IDR&=(1u<<FLAG_pin);
	
	clr_LCD_E();														//set E=0
	
	while (flag_status==0x00000080)			//keep in the loop while LCD is busy ie pin 7 = 1
	{
		set_LCD_E();						//E=1
		MASK_FLAG();						//mask pin 7 by ANDing current value with 1 on pin 7
		flag_status=MASK_FLAG();
		clr_LCD_E();						//E=0
	}
	LCD_PORT->MODER|=(1u<<(2*FLAG_pin));	//set pin 7 back, as output
}




//--------------------------Convert int to ASCII and display on LCD with analogue bar
void int_to_lcd(unsigned short ADC_readings)
{	
												//int ---> ascii ---> LCD display
	const unsigned int threshold[6] = {100000, 10000, 1000, 100, 10, 1}; 			 // threshold values for 5 digits
	unsigned int readings_to_volt = (unsigned int)(ADC_readings*7.3995);					 // 12bit ADC reading => 4095/0.1351485149 = 30300 .... where 1/0.1351485149 = 7.399267399
	unsigned char num_bank[6]; 										//set an array of chars' to be displayed on LCD
	
	cmdLCD_4bit_mask(0x01); 														// clear LCD display
	cmdLCD_4bit_mask(LCD_LINE1);												//select LCD line 1 to display
	string_putLCD("Volts  = ");										//write <Volts = > on LCD line 1

	for (int i=0; i<6; i++) 
	{		
			num_bank[i] = '0';												//assign array elements as ASCII '0'
		
											/*run loop until readings_to_volt (each digit) 
												is down to threshold value. Follow through 
												example(FTE): readings to volt = 12090, 
												therefore enter loop when threshold[i]=10000*/
			while (readings_to_volt >= threshold[i])  //stay in loop while readings_to_volt is down to threshold value's highest digit
			{
				readings_to_volt -= threshold[i]; 			//reduce readings_to_volt by threshold value (FTE: 12090-10000=02090)
				num_bank[i]++;													/*increment num_bank by '1' (num_bank is already ASCII format) */
			}																					/*(FTE: num_bank[]={'0', '1', '0', '0', '0', '0'})*/
	}													/*Repeat process for every digit*/
	
	num_bank[0] = ' ';
	num_bank[2] = '.';														//insert '.' into the number by replacing 2nd digit from left (FTE: 12090 --> '.' --> 1.090)
		unsigned int s=0;		
		while(num_bank[s]!='\0')										//run loop for the length of num_bank[] string
		{	
			putLCD_4bit(num_bank[s]); 								//send each char of the num_bank[] string to LCD display
			s++;
		}
	putLCD_4bit('V'); 													//write <V> on LCD line 1 after num_bank[] string
}	

//---------------------Writes analogue style voltage following bar
void bar_to_LCD(unsigned short ADC_readings)
{						
	cmdLCD_4bit_mask(LCD_LINE2);
																			// change to line 2 LCD display
	unsigned short comparator_threshold[16] = {2, 254, 508, 762, 1016, 1270,		//Array containing threshold values to compare against ADC_readings
																						1524, 1778, 2032, 2286, 2540,					//16 values for 16 blocks of LCD display
																						2794, 3048, 3302, 3556, 3810};
	
		//-----------------------write given number of '~' to LCD display, number of '~' are determined by the comparator_threshold values  
	if(ADC_readings<comparator_threshold[0]){string_putLCD(" ");}
	else if(ADC_readings>comparator_threshold[0] && ADC_readings<comparator_threshold[1]){string_putLCD("~               ");}
	else if(ADC_readings>comparator_threshold[1] && ADC_readings<comparator_threshold[2]){string_putLCD("~~              ");}
	else if(ADC_readings>comparator_threshold[2] && ADC_readings<comparator_threshold[3]){string_putLCD("~~~             ");}
	else if(ADC_readings>comparator_threshold[3] && ADC_readings<comparator_threshold[4]){string_putLCD("~~~~            ");}
	else if(ADC_readings>comparator_threshold[4] && ADC_readings<comparator_threshold[5]){string_putLCD("~~~~~           ");}
	else if(ADC_readings>comparator_threshold[5] && ADC_readings<comparator_threshold[6]){string_putLCD("~~~~~~          ");}
	else if(ADC_readings>comparator_threshold[6] && ADC_readings<comparator_threshold[7]){string_putLCD("~~~~~~~         ");}
	else if(ADC_readings>comparator_threshold[7] && ADC_readings<comparator_threshold[8]){string_putLCD("~~~~~~~~        ");}
	else if(ADC_readings>comparator_threshold[8] && ADC_readings<comparator_threshold[9]){string_putLCD("~~~~~~~~~       ");}
	else if(ADC_readings>comparator_threshold[9] && ADC_readings<comparator_threshold[10]){string_putLCD("~~~~~~~~~       ");}
	else if(ADC_readings>comparator_threshold[10] && ADC_readings<comparator_threshold[11]){string_putLCD("~~~~~~~~~~      ");}
	else if(ADC_readings>comparator_threshold[11] && ADC_readings<comparator_threshold[12]){string_putLCD("~~~~~~~~~~~     ");}
	else if(ADC_readings>comparator_threshold[12] && ADC_readings<comparator_threshold[13]){string_putLCD("~~~~~~~~~~~~    ");}
	else if(ADC_readings>comparator_threshold[13] && ADC_readings<comparator_threshold[14]){string_putLCD("~~~~~~~~~~~~~   ");}
	else if(ADC_readings>comparator_threshold[14] && ADC_readings<comparator_threshold[15]){string_putLCD("~~~~~~~~~~~~~~  ");}
	else if(ADC_readings>comparator_threshold[15]){string_putLCD("~~~~~~~~~~~~~~~~");}
	else{string_putLCD("~");}
}
		
//---------------------writes a string to LCD display (4-bit)
void string_putLCD(char input[])
{		
	unsigned int s=0;
	while(input[s]!='\0') //run the loop for the length of argument string (input[]) 
	{	
		putLCD_4bit(input[s]); //send each char of the argument string to LCD display
		s++;
	}
}


