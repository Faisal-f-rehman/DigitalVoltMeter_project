#include "USART.h"
#include "timInterrupt.h"
#include "ADC.h"
#include "DAC.h"


char read_usart = '0';
unsigned short state=0;
unsigned short state_to_erase=0;
unsigned short erase_state1[32];
unsigned short erase_state2[32];

void send_usart(unsigned char d)
{
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=d;		//write byte to usart data register
}

void init_USART(void)
{
	unsigned char i1,i2;

//	---------- Port clock already enabled in LCD initialisation -------------- 
//		SystemCoreClockUpdate();
//		RCC->AHB1ENR|=RCC_AHB1ENR_GPIODEN;		//usart port clock enable
	
	USART_PORT->MODER&=~(				//clear pin function bits
		(3u<<(2*USART_TX_pin))
		|(3u<<(2*USART_RX_pin))
			);
	USART_PORT->MODER|=(			//reset pin function bits (alternate function)
		(2u<<(2*USART_TX_pin))
		|(2u<<(2*USART_RX_pin))
			);
	
	i1=USART_TX_pin/8;
	i2=USART_RX_pin/8;

		// ALTERNATE FUNCTION SELECT BITS
	USART_PORT->AFR[i1]&=~(0x0f<<(4*(USART_TX_pin-(i1*8))));
	USART_PORT->AFR[i1]|=(0x07<<(4*(USART_TX_pin-(i1*8))));
	USART_PORT->AFR[i2]&=~(0x0f<<(4*(USART_RX_pin-(i2*8))));
	USART_PORT->AFR[i2]|=(0x07<<(4*(USART_RX_pin-(i2*8))));
	
	RCC->APB1ENR|=RCC_APB1ENR_USART3EN;		//usart clock enable
	USART_MODULE->CR1|=(		//USART CONFIG
			//USART_CR1_RXNEIE
			USART_CR1_TE		//transmit enable
			|USART_CR1_RE		//receive enable
			|USART_CR1_UE		//usart main enable bit
				);
	USART_MODULE->BRR=SystemCoreClock/BAUDRATE;		//set baud rate
	send_usart_2arg("2","J"); //clear terminal screen
	send_usart_2arg("H","");	 //move cursor to home position (row=1, column=1)
}
//------------------------------------------------------------


//--------------------sends a string to terminal screen:
void send_usart_string(char input[] /*,char input[]*/)
{
	unsigned int s=0;	
	while(input[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=input[s];		//write byte to usart data register
		s++;		
	}
}
//--------------------------------------------------------------


//------------------sends two argument ANSI/VT100 ESC[2J command to terminal
void send_usart_2arg(char num[], char cmd[])
{
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x1B;										//Send hex for ASCII "Escape" to data register
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x5B;										//Send hex for ASCII "[" to data register

	unsigned int s=0;	
	while(num[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=num[s];		//write byte to usart data register
		s++;		
	}
	s=0;

	while(cmd[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=cmd[s];									//write byte to usart data register
		s++;		
	}
}
//-----------------------------------------------------------------------------

//------------------sends cursor position command
void send_usart_curPos(char row[], char col[])
{	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x1B;										//Send hex for ASCII "Escape" to data register
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x5B;										//Send hex for ASCII "[" to data register

	unsigned int s=0;	
	while(row[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=row[s];		//write byte to usart data register
		s++;		
	}
	s=0;
	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x3B;										//Send hex for ASCII ";" to data register
	
	while(col[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=col[s];		//write byte to usart data register
		s++;		
	}
	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x48;										//Send hex for ASCII "H" to data register
}
//-----------------------------------------------------------------------------

//------------------sends cursor position command
void send_usart_curPos_int(unsigned short row, unsigned short col)
{	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x1B;										//Send hex for ASCII "Escape" to data register
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x5B;										//Send hex for ASCII "[" to data register

	int_to_USART(row);
	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x3B;										//Send hex for ASCII ";" to data register
	
	int_to_USART(col);
	
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x48;										//Send hex for ASCII "H" to data register
}
//-----------------------------------------------------------------------------

//--------------------------Convert int to ASCII and display on terminal
void volt_to_USART(unsigned short ADC_readings)
{	
												//int ---> ascii ---> terminal display
	const unsigned int threshold[6] = {100000, 10000, 1000, 100, 10, 1}; 			 // threshold values for 5 digits
	unsigned int readings_to_volt = (unsigned int)(ADC_readings*7.3995);			 // 12bit ADC reading => 4095/0.1351485149 = 30300 .... where 1/0.1351485149 = 7.399267399
	unsigned char num_bank[6]; 						//set an array of chars' to be displayed on terminal
	
	send_usart_string("VOLTAGE  = ");			//write <VOLTAGE = > on terminal

	for (int i=0; i<6; i++) 
	{		
			num_bank[i] = '0';								//assign array elements as ASCII '0'
		
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
			send_usart(num_bank[s]); 									//send each char of the num_bank[] string to terminal screen
			s++;
		}
	send_usart('V'); 															//write <V> on terminal screen after num_bank[] string
}	
//--------------------------------------------------------------------

//-----------------------sends 3 argument based command to terminal:
void send_usart_3arg(char ascii1[], char ascii2[], char ascii3[])
{
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x1B;										//Send hex for ASCII "Escape" to data register
	while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
	USART_MODULE->DR=0x5B;										//Send hex for ASCII "[" to data register

	unsigned short s=0;	
	while(ascii1[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=ascii1[s];								//write byte to usart data register
		s++;		
	}
	s=0;

	while(ascii2[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=ascii2[s];								//write byte to usart data register
		s++;		
	}
	s=0;
	
	while(ascii3[s]!='\0')
	{	
		while(!(USART_MODULE->SR&USART_SR_TC));		//wait for transmission complete
		USART_MODULE->DR=ascii3[s];								//write byte to usart data register
		s++;		
	}
	
}

//----------------move cursor to home position
void usart_cur_home(void)
{
	send_usart_2arg("1","B"); //move cursor down 1 row
	send_usart_2arg("H","");	 //move cursor to home position (row=1, column=1)
}
//--------------------------------------------------------------------

//Draws the top frame on terminal display
void usart_frame(void)
{
	send_usart_3arg("3","6","m");	//change text colour cyan
	send_usart_string("||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
	send_usart_curPos("3","1"); 	//row=3, column=1
	send_usart_string("||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
	send_usart_3arg("3","9","m");	//change text colour default
}
//--------------------------------------------------------------------

//Only writes modes available on terminal screen, does not gives the option to select 
void usart_mode_sel(void)
{
	send_usart_curPos("2","1");		//row=2, column=1
	send_usart_3arg("3","6","m");	//change text colour cyan
	send_usart_string("||||| PRESS NUMBER KEY TO SELECT MODE:");
	
	
	//----DC----
	send_usart_2arg("2","C");				//move cursor forward by 2 spaces
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","2","m");		//change text colour green
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("1");					//write ASCII character on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","2","m");		//change text colour green
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("     DC  ");	//write ASCII characters on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","9","m");		//change text colour default
	
	//----SINEWAVE----
	send_usart_2arg("3","C");				//move cursor forward by 3 spaces
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","4","m");		//change text colour blue
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("2");					//write ASCII character on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","4","m");		//change text colour blue
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("   SINEWAVE");//write ASCII characters on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","9","m");		//change text colour default
	
	//----SQUAREWAVE----
	send_usart_2arg("3","C");				//move cursor forward by 3 spaces
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","3","m");		//change text colour yellow
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("3");					//write ASCII character on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","3","m");		//change text colour yellow
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("  SQUAREWAVE");//write ASCII characters on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","9","m");		//change text colour default
	
		//----TRIANGULARWAVE----
	send_usart_2arg("2","C");				//move cursor forward by 3 spaces
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","5","m");		//change text colour magenta
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("4");					//write ASCII character on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart('|');								//write ASCII character on terminal screen
	send_usart_3arg("3","5","m");		//change text colour magenta
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string(" TRIANGULARWAVE ");//write ASCII characters on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("4","9","m");		//change background colour default
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart_string("||");				//write ASCII characters on terminal screen
	send_usart_3arg("3","9","m");		//change text colour default
	
	//----current mode----
	send_usart_2arg("2","C");				//move cursor forward by 3 spaces
	send_usart_3arg("3","9","m");		//change text colour default
	send_usart_3arg("4","1","m");		//change background colour red
	send_usart_2arg("1","m");				//bold/bright text
	send_usart_string("MODE SELECTED:"); //write ASCII characters on terminal screen
	send_usart_2arg("2","m");				//dim text
	send_usart_3arg("4","9","m");		//change background colour default
	send_usart_3arg("3","6","m");		//change text colour cyan
	send_usart_string("                      ||||||||||");//write ASCII characters on terminal screen
	send_usart_3arg("3","9","m");		//change text colour default
}

//--------------------------------------------------------------------

//sends ADC value to usart and writes on terminal screen
void ADC_volt_to_usart(unsigned short int_to_ascii)
{
	send_usart_curPos("4", "1");		//send usart cursor position to row=4, column=1
	volt_to_USART(int_to_ascii);		//send running voltage value to terminal screen
}
//--------------------------------------------------------------------

//This function gets triggered by variable usart_in in timer 4 handler if a character has been received
//Once triggered it reads the data received from the data register and writes it back on to the terminal.
void receive_usart(void)
{
	//check trigger
	if(usart_in>=1)
	{		
		read_usart=USART_MODULE->DR; 							//assign the data received to read_usart
	}
	
	send_usart_curPos("2","129");								//set cursor position
	while(!(USART_MODULE->SR&USART_SR_TC));			//wait for transmission complete
	USART_MODULE->DR=read_usart;								//write back the data received on to terminal on the row and column selected above
	send_usart_curPos("2","130");								//set cursor position
	send_usart_string("                   ");		//write spaces to terminal to manually erase columns in front of the data written above, erases 19 columns and allows 1 to write to 
	usart_in=0;																	//reset trigger variable
	
	if(read_usart>'4' || read_usart <= '0')			//enters if anything but 1, 2, 3 or 4 is received from the terminal
	{
		send_usart_curPos("2","129");							//set cursor position
		send_usart_string("SELECT 1, 2, 3 OR 4");	//write on terminal to ask user to press 1, 2, 3 or 4
	} 
}
//--------------------------------------------------------------------

//writes adc line and bar DC representation of voltage on terminal
void ADC_bar_USART(void)
{
	unsigned short runner_adc = 0;
	short holder_adc = 0;
	
	
	send_usart_3arg("3","2","m");						//change text colour green
	runner_adc=read_adc();									//assign adc value to variable 
	holder_adc=runner_adc/bar_resolution;		//divide adc value by bar_resolution, bar_resolution is set on top as a #define
	send_usart_curPos("8","1");							//SET cursor position to row = 8, column = 1
	while(holder_adc>=0)										//enter loop for the number of times defined by adc value/bar_resolution
	{
		send_usart('/');											//send characterto terminal, this character makes the bar			
		send_usart_2arg("1","C");							//move cursor right 1 column
		send_usart_2arg("K","");							//Erase from the current cursor position to the end of the line
		holder_adc--;													//decrements holder_adc until it reaches zero
	}
	send_usart_curPos("9","1");							//set cursor position
	//next line makes the scale under bar representation
	send_usart_string("_0___________________________________________________________________________________________________________________________________________3.3V");
	
	//-----------------Line representation
	//between row 12 and 30 where row 30 is voltage = 0
	//divide adc value to give 30-12 = 18 => 4095/227.5 = 18, then deduct the calculated value by 18 and times it by -1 to reverse the position for the terminal 
	holder_adc=runner_adc/227.5;								//divide adc value by 227.5
	holder_adc = ((-1)*(holder_adc-18))+12;	//reverse number of rows and shift it up from 0 to 12
	send_usart_curPos_int(holder_adc,1);				//set cursor position to calculated holder_adc value
	//erase_state_machine(holder_adc);
	//send_usart_string("________________________________________________________________________________________________________________________________________________________________");
	send_usart_3arg("3","9","m");								//change text colour default
}

void int_to_USART(unsigned short int_value)
{	
												//int ---> ascii ---> terminal display
	const unsigned int threshold[3] = {100, 10, 1}; 			 // threshold values for 5 digits
	unsigned char num_bank[3]; 						//set an array of chars' to be displayed on terminal

	for (int i=0; i<3; i++) 
	{		
			num_bank[i] = '0';								//assign array elements as ASCII '0'
		
											/*run loop until readings_to_volt (each digit) 
												is down to threshold value. Follow through 
												example(FTE): readings to volt = 12090, 
												therefore enter loop when threshold[i]=10000*/
			while (int_value >= threshold[i])  //stay in loop while readings_to_volt is down to threshold value's highest digit
			{
				int_value -= threshold[i]; 			//reduce readings_to_volt by threshold value (FTE: 12090-10000=02090)
				num_bank[i]++;													/*increment num_bank by '1' (num_bank is already ASCII format) */
			}																					/*(FTE: num_bank[]={'0', '1', '0', '0', '0', '0'})*/
	}													/*Repeat process for every digit*/
	
		unsigned int s=0;		
		while(num_bank[s]!='\0')										//run loop for the length of num_bank[] string
		{	
			send_usart(num_bank[s]); 									//send each char of the num_bank[] string to terminal screen
			s++;
		}
}	

//sends sinewave to usart
void sine_to_usart(void)
{
		unsigned short runner_adc = 0;
		unsigned short sinewave_usart[32];
		
		//holds 32 values of a sinewave calculated manually on excel, formula: sin(x*2pi/32) where x is 1 to 32, scaled up by multiplying with 1000 to avoid floating points
		signed short sinewave_bank[32]={195, 383, 556, 707, 831, 924, 981, 1000,
																		981, 924, 831, 707, 556, 383, 195, 0,
																	 -195,-383,-556,-707,-831,-924,-981,-1000,
																	 -981,-924,-831,-707,-556,-383,-195, 0};
		
	
		runner_adc=read_adc();																											//assign adc value to runner_adc
		for(unsigned short i=0;i<=31;i++)																						//run loop 32 times
		{																																						//multiply sinewave bank by 0.001 to scale down by 1000 then multiply adc value by sinewave_bank/1000
			sinewave_usart[i]=(((sinewave_bank[i]*0.001)*(runner_adc*0.01))*0.5)+30;	//then set sinewave point 0 by dividing ADC data into half and shift wave up by 30 to keep the wave positive
		}
		
		erase_state_machine(sinewave_usart);						//erase last sinewave
		
		//send 1st cycle of sinewave
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(sinewave_usart[i],i);		//set cursor position row = sinewave_bank[0 to 31] and column = 0 to 31 
			send_usart('O');															
		}
		//send 2nd cycle of sinewave
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(sinewave_usart[i],i+32);//set cursor position row = sinewave_bank[0 to 31] and column = 32 to 63 
			send_usart('O');
		}
		//send 3rd cycle of sinewave
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(sinewave_usart[i],i+64);//set cursor position row = sinewave_bank[0 to 31] and column = 64 to 95 
			send_usart('O');
		}
		//send 4th cycle of sinewave
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(sinewave_usart[i],i+96);//set cursor position row = sinewave_bank[0 to 31] and column = 96 to 127
			send_usart('O');
		}
		//send 5th cycle of sinewave
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(sinewave_usart[i],i+128);//set cursor position row = sinewave_bank[0 to 31] and column = 128 to 159 
			send_usart('O');
		}	
}

void square_to_usart(void)
{
	unsigned short squarewave_master[32];
	unsigned short squarewave_slave[32];
	unsigned short runner_adc = 0;
	//stores 32 values first half = 0 and the second half = 1
	unsigned short squarewave_bank[32]={0,0,0,0,0,0,0,0,
																				0,0,0,0,0,0,0,0,
																				1,1,1,1,1,1,1,1,
																				1,1,1,1,1,1,1,1};
	//cursor positions for squarewave on usart required between rows = 30 and 11, where 30 = 0volts and 11 = 3.3volts
	//formula: ((squarewave_bank[]*adc value)*1.45/120)+30) - adc value/60    ------------> workings in excel
	runner_adc=read_adc()*0.01666667;																						//divide adc value by 60
	for(unsigned short i=0;i<=31;i++)
	{
		squarewave_slave[i]=(((squarewave_bank[i]*read_adc())*0.0120833333)+30);	//perform the following section of the formula: ((squarewave_bank[]*adc value)*1.45/120)+30
		squarewave_master[i]=squarewave_slave[i];																	//store all 32 values from slave to master
	}		
	for(unsigned short i=16;i<=31;i++)
	{
		squarewave_master[i]=squarewave_slave[i]-runner_adc;											//apply the deduction to only the second half of the master array 
	}
	
	erase_state_machine(squarewave_master);																			//erase last squarewave
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(squarewave_master[i],i);														//send calculated cursor positions row = master array and column = 0 to 31
		send_usart('-');																													//send character to master array cursor position
		
//------------vertical lines of squarewave
//		if(i>=15 && i<16)
//		{
//			for(unsigned short j=0;j<((read_adc()*0.0041666667)-0.5);j++)
//			{
//				send_usart_2arg("1","A");			//move cursor up 1 row
//				send_usart_2arg("1","D");			//move cursor left 1 column
//				send_usart('|');							//print ASCII '|' to at cursor position
//			}
//			send_usart_curPos_int(squarewave_master[15],15);
//				for(unsigned short j=0;j<((read_adc()*0.0041666667)-0.5);j++)
//			{
//				send_usart_2arg("1","A");
//				send_usart_2arg("1","D");
//				send_usart(' ');
//			}
//		}
	}
	
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(squarewave_master[i],i+32);//send calculated cursor positions row = master array and column = 32 to 63
		send_usart('-');
	}
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(squarewave_master[i],i+64);//send calculated cursor positions row = master array and column = 64 to 95
		send_usart('-');
	}
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(squarewave_master[i],i+96);//send calculated cursor positions row = master array and column = 96 to 127
		send_usart('-');
	}
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(squarewave_master[i],i+128);//send calculated cursor positions row = master array and column = 128 to 159
		send_usart('-');
	}
}


//sends triangularwave to usart
void tri_to_usart(void)
{
	unsigned short tri_to_usart_bank[32];
	unsigned short tri_inverter;
	unsigned short seventeen_to_zero = 17;
	unsigned short thirtyOne_to_eighteen = 31;

	
	//the next two for loops takes wave values from DAC.c and turn them upside down
	for(unsigned short i=0;i<=17;i++) 											//run loop for array positions 0 to 17
	{
		tri_to_usart_bank[i]=triWave_bank[seventeen_to_zero]; //takes first half of triWave_bank from DAC.c and stores in tri_to_usart_bank in reverse order
		if(seventeen_to_zero<=0){seventeen_to_zero=17;}				//reset variable back to 17 if limit reached
		seventeen_to_zero--;																	//decrement variable by 1 
	}
	
	for(unsigned short i=18;i<=31;i++)
	{
		tri_to_usart_bank[i]=triWave_bank[thirtyOne_to_eighteen]; //takes second half of triWave_bank from DAC.c and stores in tri_to_usart_bank in reverse order
		if(thirtyOne_to_eighteen<=18){thirtyOne_to_eighteen=31;}	//reset variable back to 31 if limit reached
		thirtyOne_to_eighteen--;																	//decrement variable by 1
	}
	
	//this for loop inverts the upside down wave and pushes it up 
	for(unsigned short i=0;i<=31;i++)
	{
		
		tri_inverter=read_adc()*0.0048780;																					//divide adc value by 205 ie max value = 4095/205 = 20
		tri_to_usart_bank[i]=((tri_to_usart_bank[i]*0.0048780)-tri_inverter)+30;		//formula: at max values ---> ((127/205)-0/205)+30 = 30, at min value ---> ((0/205)-4095/205)+30 = 10
	}	

	erase_state_machine(tri_to_usart_bank);																				//erase last triangular wave
	
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(tri_to_usart_bank[i],i);															//send calculated cursor positions row = tri_to_usart_bank and column = 0 to 31
		send_usart('-');
	}
	
		for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(tri_to_usart_bank[i],i+32);														//send calculated cursor positions row = tri_to_usart_bank and column = 32 to 63
		send_usart('-');
	}
	
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(tri_to_usart_bank[i],i+64);														//send calculated cursor positions row = tri_to_usart_bank and column = 64 to 95
		send_usart('-');
	}
	
	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(tri_to_usart_bank[i],i+96);														//send calculated cursor positions row = tri_to_usart_bank and column = 96 to 127
		send_usart('-');
	}

	for(unsigned short i=0;i<=31;i++)
	{
		send_usart_curPos_int(tri_to_usart_bank[i],i+128);													//send calculated cursor positions row = tri_to_usart_bank and column = 128 to 159
		send_usart('-');
	}	
}

void usart_switch_wave(void)
{
	switch(read_usart)
	{
		case '1':
			send_usart_2arg("1","m");		//bold/bright text
			send_usart_3arg("3","2","m");								//change text colour green  
			send_usart_curPos("6","69");								//move cursor position to row=11, column=72
			send_usart_2arg("2","K");										//erase the whole line
			send_usart_string("DC BAR REPRESENTATION");	//write mode type on terminal screen 
			send_usart_3arg("3","9","m");								//change text colour default
			send_usart_2arg("2","m");										//dim text
			send_usart_curPos("10","1");								//move cursor position to row=10, column=1
			send_usart_2arg("J","");										//erase screen under current row
			break;
		case '2':
			send_usart_2arg("1","m");							//bold/bright text
			send_usart_3arg("3","4","m");					//change text colour blue
			send_usart_curPos("11","69");					//move cursor position to row=11, column=72
			send_usart_2arg("2","K");							//erase the whole line
			send_usart_string("SINEWAVE");				//write mode type on terminal screen
			sine_to_usart();									//send sinewave to usart
			send_usart_3arg("3","9","m");					//change text colour default
			send_usart_2arg("2","m");							//dim text
			break;
		case '3':
			send_usart_2arg("1","m");							//bold/bright text
			send_usart_3arg("3","3","m");					//change text colour yellow  
			send_usart_curPos("11","69");					//move cursor position to row=11, column=72
			send_usart_2arg("2","K");							//erase the whole line
			send_usart_string("SQUAREWAVE");			//write mode type on terminal screen 
			square_to_usart();								//send squarewave to usart
			send_usart_3arg("3","9","m");					//change text colour default
			send_usart_2arg("2","m");							//dim text
			break;
		case '4':
			send_usart_2arg("1","m");							//bold/bright text
			send_usart_3arg("3","5","m");					//change text colour magenta  
			send_usart_curPos("11","65");					//move cursor position to row=11, column=72
			send_usart_2arg("2","K");							//erase the whole line
			send_usart_string("TRIANGULARWAVE");	//write mode type on terminal screen 
			tri_to_usart();										//send triangular wave to usart
			send_usart_3arg("3","9","m");					//change text colour default
			send_usart_2arg("2","m");							//dim text
			break;
	}
}


//erases sine, square and triangular wave
void erase_state_machine(unsigned short x[])
{
	state++; 																							//variable state declared as zero on top which is incremented by 1 here
	switch (state)																				//switch statement allows 2 states to store values at alternating cycles so that the value from the last cycle can be kept
	{
		case 1:
			for(unsigned short i=0;i<=31;i++)
			{
				erase_state1[i]=x[i];														//stores value at 1st cycle
			}
			break;
		case 2:
			for(unsigned short i=0;i<=31;i++)
			{
				erase_state2[i]=x[i];														//stores value at 2nd cycle
			}
			state=0;																					//reset state to zero to allow entry to state 1 at next entry
			break;
	}	
	
	//the following 2 if statements allows erase cycles to run 1 clock cycle behind current cycle
	if(state_to_erase <= 1)															//variable state_to_erase declared as zero on top of page and is incremented at the end to skip one clock cycle
	{
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(erase_state1[i],i);				//send last clock cycle's cursor positions of wave to terminal row = erase_state1 and column = 0 to 31
			send_usart(' ');																//write space at the cursor position to erase
			send_usart_curPos_int(erase_state1[i],i+32);		//send last clock cycle's cursor positions of wave to terminal row = erase_state1 and column = 32 to 63
			send_usart(' ');
			send_usart_curPos_int(erase_state1[i],i+64);		//send last clock cycle's cursor positions of wave to terminal row = erase_state1 and column = 64 to 95
			send_usart(' ');
			send_usart_curPos_int(erase_state1[i],i+96);		//send last clock cycle's cursor positions of wave to terminal row = erase_state1 and column = 96 to 127
			send_usart(' ');
			send_usart_curPos_int(erase_state1[i],i+128);		//send last clock cycle's cursor positions of wave to terminal row = erase_state1 and column = 128 to 159
			send_usart(' ');
		}
	}
	
	if(state_to_erase >= 2)
	{
		for(unsigned short i=0;i<=31;i++)
		{
			send_usart_curPos_int(erase_state2[i],i);				//send last clock cycle's cursor positions of wave to terminal row = erase_state2 and column = 0 to 31
			send_usart(' ');
			send_usart_curPos_int(erase_state2[i],i+32);		//send last clock cycle's cursor positions of wave to terminal row = erase_state2 and column = 32 to 63
			send_usart(' ');
			send_usart_curPos_int(erase_state2[i],i+64);		//send last clock cycle's cursor positions of wave to terminal row = erase_state2 and column = 64 to 95
			send_usart(' ');
			send_usart_curPos_int(erase_state2[i],i+96);		//send last clock cycle's cursor positions of wave to terminal row = erase_state2 and column = 96 to 127
			send_usart(' ');
			send_usart_curPos_int(erase_state2[i],i+128);		//send last clock cycle's cursor positions of wave to terminal row = erase_state2 and column = 128 to 159
			send_usart(' ');
		}
		state_to_erase = 0;																//reset state_to_erase back to zero to allow next cycle to use erase_state1
	}
	state_to_erase++;																		//increment state_to_erase by 1
}

void usart_delay_us(unsigned int us)		//argument is approximate number of micro-seconds to delay
{
	unsigned char i;
	while(us--)
	{
		for(i=0; i<SystemCoreClock/4000000; i++);
	}
}
