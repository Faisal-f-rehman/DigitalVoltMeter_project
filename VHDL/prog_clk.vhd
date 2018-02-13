library ieee; 
use ieee.std_logic_1164.all; 
use ieee.numeric_std.all; 

entity prog_clk is 
generic (clk_div : integer := 0); 
port 
	( 
	clk 		 : in std_ulogic; 
	mosi_data : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	--reset 	 : in std_ulogic; 
	count 	 : out std_ulogic_vector(20 downto 0); 
	rco 		 : out std_ulogic 
	); 
end entity prog_clk; 

architecture progressive_clk of prog_clk is 
begin 
	p0: process (clk) is -- sensitivity list 
		variable k : integer := 0; --unsigned(20 downto 0); -- counter state variable (not port) 
		variable s : unsigned(0 downto 0); -- clock state - single bit variable in this case
		variable	mosi_holder : integer:= 0;
		variable mosi_dif : integer;
		variable clk_div_holder: integer:= 0; 
		begin 
--			if reset = '1' then --asychronous reset 
--				k := clk_div; --set initial value 
--				s := "0"; 
--				rco <= '0'; 
--				clk_div_holder := clk_div;
--				mosi_dif:= to_integer(unsigned(std_logic_vector(unsigned(mosi_data)))) - mosi_holder;
--				mosi_holder:= to_integer(unsigned(std_logic_vector(unsigned(mosi_data))));
									
					
--				if mosi_dif < 12 then
--					clk_div_holder := clk_div_holder + 5000;
--				elsif mosi_dif > 12 then
--					clk_div_holder := clk_div_holder - 5000;
--				else
--					clk_div_holder := clk_div_holder;
--				end if;
--				
--				if (clk_div_holder > 2097150) then
--					clk_div_holder := 2097150;
--				elsif(clk_div_holder < clk_div+1) then
--					clk_div_holder := clk_div;
--				end if;
			
				
			if rising_edge(clk) then 
				if (k = 2097152) then --detected reaching last state
					k := clk_div; 
					s := not s; --toggle clock state 
					rco <= std_ulogic_vector(s)(0); --map clock state to output 
				else 
				k := k + 1; --increment counter 
				end if; 
			end if; 
		count <= std_ulogic_vector(to_unsigned(k,21)); --map counter state to outputs 
	end process p0; 
end architecture progressive_clk; 
