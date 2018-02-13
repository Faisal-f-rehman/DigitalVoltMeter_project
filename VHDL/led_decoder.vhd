library ieee; 
use ieee.std_logic_1164.all; 
use ieee.numeric_std.all; 

entity led_decoder is 

GENERIC(vec_width : INTEGER := 8);  --data width in bits
	 
port 
(
clk	: IN 	STD_LOGIC;										--	clk input
mosi	: IN 	STD_LOGIC_VECTOR(vec_width-1 DOWNTO 0);--	this input is to read data from the slave's rx_data(7 DOWNTO 0) output
led 	: out STD_LOGIC_VECTOR(vec_width-1 DOWNTO 0) -- 8 output bits to send to the leds
);

end entity led_decoder; 



architecture led_decoder_1 of led_decoder is 

-- Declare signals:
SIGNAL led_buf		:	STD_LOGIC_VECTOR(vec_width-1 downto 0):= (OTHERS => '0'); -- 8 bits led buffer signal
SIGNAL mosi_div	:	STD_LOGIC_VECTOR(vec_width-1 downto 0):= (OTHERS => '0'); -- 8 bits signal used to hold data from input mosi after division
SIGNAL mosi_led	:	STD_LOGIC_VECTOR(2 downto 0):= (OTHERS => '0'); -- 3 bits signal used to hold 3 MSBs from mosi
SIGNAL count_lim 	: 	integer;	-- Signal of type integer used for holding value of mosi at 8 different steps															

begin 

	P0: PROCESS(clk, count_lim) 			-- start process P0, sensitivity clk and count_lim
	
	--Declare Variables
	VARIABLE count : integer := 0;		-- variable of type integer, counts no. of rising edge clicks for the PWM	
	VARIABLE count_scl : integer;			-- variable of type integer, holds value of adjustment for PWM
	
	BEGIN
			mosi_led<=mosi(vec_width-1 DOWNTO vec_width-3);						--assign only 3 MSBs of mosi to mosi_led
			mosi_div<= std_logic_vector(shift_right(unsigned(mosi),5));		-- Divide data from mosi by 32 to get 8 equal steps (255/32 = 8) and assign it to mosi_div
								
			
			-- Depending on the state of mosi_div led_buf is assigned a value, this is used to select leds
			CASE mosi_led(2 DOWNTO 0) IS  			-- BEGIN case statement with dependance (expression), mosi_div of 3bits
			WHEN "000" => led_buf <= "00000000";	-- if mosi_div is 0 led_buf = 0
			WHEN "001" => led_buf <= "00000001";	-- if mosi_div is 1 led_buf = 1
			WHEN "010" => led_buf <= "00000011";	-- if mosi_div is 2 led_buf = 3
			WHEN "011" => led_buf <= "00000111";	-- if mosi_div is 3 led_buf = 7
			WHEN "100" => led_buf <= "00001111";	-- if mosi_div is 4 led_buf = 15
			WHEN "101" => led_buf <= "00011111";	-- if mosi_div is 5 led_buf = 31
			WHEN "110" => led_buf <= "00111111";	-- if mosi_div is 6 led_buf = 63
			WHEN "111" => led_buf <= "01111111";	-- if mosi_div is 7 led_buf = 127
			WHEN OTHERS => NULL;							-- if mosi_div is any other value led_buf = null
			END CASE;										-- END case statement
			
			-- Depending on the state of mosi_div count_lim is assigned a value, this is used for resetting PWM when ever the leds change state
			CASE mosi_div(2 DOWNTO 0) IS  			-- BEGIN case statement with dependance (expression), mosi_div of 3bits
			WHEN "000" => count_lim <= 0;				-- if mosi_div is 0 count_lim = 0
			WHEN "001" => count_lim <= 32;			-- if mosi_div is 1 count_lim = 32
			WHEN "010" => count_lim <= 64;			-- if mosi_div is 2 count_lim = 64
			WHEN "011" => count_lim <= 96;			-- if mosi_div is 3 count_lim = 96
			WHEN "100" => count_lim <= 128;			-- if mosi_div is 4 count_lim = 128
			WHEN "101" => count_lim <= 160;			-- if mosi_div is 5 count_lim = 160
			WHEN "110" => count_lim <= 192;			-- if mosi_div is 6 count_lim = 192
			WHEN "111" => count_lim <= 224;			-- if mosi_div is 7 count_lim = 224
			WHEN OTHERS => NULL;							-- if mosi_div is any other value count_lim = null
			END CASE;										-- END case statement
			
			count_scl := to_integer(unsigned(std_logic_vector(unsigned(mosi)))) - count_lim ;	-- Convert mosi of type std_logic_vector to integer, deduct count_lim from it 
																															-- to keep value between 0 and 32 and assign this value to count_scl
				
			
			--------------------------------- PWM AND LED STATES ----------------------------------------
			-- At every rising edge of the clock(clk), process the if statement
			IF RISING_EDGE(clk) THEN
				
				IF count > 249 - count_scl THEN	-- if count exceeds 250 - count_scl enter if statement and turn appropriate leds on, where count_scl ranges between 0 and 32 		
					led<=led_buf(6 DOWNTO 0) & '1';		-- Only read the 6 least significant bits of led_buf and join bit = 1 to the LSB and assign it to led, 
					count := count +1;						-- eg if led_buf is 00011111 then led = 00111111
				ELSE
					led<=led_buf;								-- keep only led_buf value leds on, eg if led_buff = 00011111 keep 00011111 on, this turns off only the MSB LED
					count := count + 1;						-- increase count by 1
				END IF;
				
				IF count = 250 THEN							-- Limit the PWM cycle to 250 counts, if count reaches 250, 
					count := 0;									-- reset count to zero
				END IF;
			END IF;
	
		
	END PROCESS P0;								-- END process P0
end architecture led_decoder_1; 				-- END architecture



--------------------------------------- Code not used ---------------------------------------------
                         ------ OLD algorathims that didnt work -------

--SIGNAL rx_state: STD_LOGIC_VECTOR(2 DOWNTO 0) := (OTHERS => '0');  --transmit buffer
--	VARIABLE mosi_div_x : integer:= 0;
--	VARIABLE mosi_div_smp : integer;
--	VARIABLE mosi_div_dif : integer;
--	VARIABLE mosi_x : integer:= 0;
--	VARIABLE mosi_smp : integer;
--	VARIABLE mosi_dif : integer;

--			rx_cal := to_integer(unsigned(mosi))/511;
--			led_buf <= STD_LOGIC_VECTOR(to_unsigned(rx_cal,vec_width-1));
--			rx_cal := mosi_led/511;
			
--			CASE rx_cal IS  
--			WHEN 0 => led_buf <= "000000000101";
--			WHEN 1 => led_buf <= "000000000011";
--			WHEN 2 => led_buf <= "000000000111";
--			WHEN 3 => led_buf <= "000000001111";
--			WHEN 4 => led_buf <= "000000011111";
--			WHEN 5 => led_buf <= "000000111111";
--			WHEN 6 => led_buf <= "000001111111";
--			WHEN 7 => led_buf <= "000011111111";
--			WHEN OTHERS => NULL;
--			END CASE;
			
--			mosi_x(0) := mosi_led; 
--			rx_cal := to_integer(unsigned(mosi_x))/511;
--			rx_state <= STD_LOGIC_VECTOR(to_unsigned(rx_cal,3));
			
			
			


--			IF RISING_EDGE(prog_clk) THEN
--			
--				IF mosi_x > 1 THEN
--					mosi_smp := to_integer(unsigned(std_logic_vector(unsigned(mosi))));
--					mosi_x := 0;
--				ELSE
--					mosi_dif:= to_integer(unsigned(std_logic_vector(unsigned(mosi)))) - mosi_smp;
--					mosi_x := mosi_x + 1;
--				END IF;
--			END IF;
--						
--				if mosi_dif < 12 then
--					count_lim := count_lim - 1;
--				elsif mosi_dif > 12 then
--					count_lim := count_lim + 1;
--				else
--					count_lim := count_lim;
--				end if;
--				
--				if (count_lim > 19) then
--					count_lim := 20;
--				elsif(count_lim < 1) then
--					count_lim := 0;
--				end if;			
				
				
				
				
				
--				IF mosi_div_x > 0 THEN
--					mosi_div_smp := to_integer(unsigned(std_logic_vector(unsigned(mosi_div))));
--					mosi_div_x := 0;
--				ELSE
--					mosi_div_dif:= to_integer(unsigned(std_logic_vector(unsigned(mosi_div)))) - mosi_div_smp;
--					mosi_div_x := mosi_div_x + 1;
--				END IF;
--				
--				IF mosi_x > 1 THEN
--					mosi_smp := to_integer(unsigned(std_logic_vector(unsigned(mosi))));
--					mosi_x := 0;
--				ELSE
--					mosi_dif:= to_integer(unsigned(std_logic_vector(unsigned(mosi)))) - mosi_smp;
--					mosi_x := mosi_x + 1;
--				END IF;
--				
--				if mosi_div_dif < 0 then count_lim <= 15;
--				elsif mosi_div_dif > 0 then count_lim <= 0;
----				elsif count_lim > 13 then count_lim <= 15;
----				elsif count_lim < 2 then count_lim <= 0;
--				elsif mosi_dif < 0 then count_lim <= - 1;
--				elsif mosi_dif > 0 then count_lim <= 1;
--				end if;
--				--count_lim <=  to_integer(unsigned(std_logic_vector(unsigned(mosi))));
----				count_scl :=  count_scl + count_lim;
----				if count_scl < 1 then count_scl := 0;
----				elsif count_scl > 14 then count_scl := 15;
----				end if;
