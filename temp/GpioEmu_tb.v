module GpioEmu_tb;

	initial begin
		$dumpfile("GpioEmu.vcd");
		$dumpvars(0, GpioEmu_tb);
	end	
	
// Sygnały wejściowe
reg n_reset = 1; 
reg [31:0] sdata_in; 
reg [15:0] saddress;
reg [31:0] gpio_in=0;
reg gpio_latch=0;
reg srd=0;
reg swr=0;

reg clk=0;

//Zdefiniowane parametry
parameter max_SIZE_INPUT = 24;
parameter max_SIZE_OUTPUT = 32;
parameter total_SIZE = 48;

// Sygnały wyjściowe
wire [31:0] sdata_out;
wire [31:0] gpio_out;
wire [31:0] gpio_in_s_insp;


gpioemu gpioemu(n_reset, saddress, srd, swr, sdata_in, sdata_out, gpio_in, gpio_latch, gpio_out, clk, gpio_in_s_insp);



always begin #1 clk = ~clk; end


initial begin
    clk = 0;

   // reset
	# 5 n_reset = 0;
	# 5 n_reset = 1;

	// Testowanie prostych liczb całkowitych, których wynik mieści się na 32 bitach
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h00000005;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h00000002;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika 
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// Testowanie mnożenia przez zero
	
	#20
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h00000000;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h00000001;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
	
	
	// Testowanie prostych liczb całkowitych, których wynik mieści się na 32 bitach po mnożeniu przez zero (aby sprawdzić, czy nie wywołało ono jakichś błędów w otrzymywanych wynikach)
	
	#20
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h00000002;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h00000003;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// Testowanie w przypadku przepełnienia 
	
	#20
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h00FFFFFF;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h00FFFFFF;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// Testowanie przypadku, gdy na A1 jest zbyt duża wartość z sdata_in
	
	#20
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h0FFFFFF1;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h00000005;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
	
	
	// Testowanie przypadku, gdy na A2 jest zbyt duża wartość z sdata_in
	
	#20
	
	// ustalamy wartość wejściową zmiennej A1
	# 5 sdata_in = 32'h00000005;
	# 5 saddress = 16'h220;
	# 5 swr = 1;
	# 5 swr = 0;
	
	// ustalamy wartość wejściową zmiennej A2 i rozpoczynamy pracę modułu
	# 5 sdata_in = 32'h0FFFFFF1;
	# 5 saddress = 16'h228;
	# 5 swr = 1;
	# 5 swr = 0;
	
	#20
	
	// odczytujemy wynik operacji
	# 5 saddress = 16'h230;
	# 5 srd = 1;
	# 5 srd = 0;

	// odczytujemy wartość licznika
	# 5 saddress = 16'h238;
	# 5 srd = 1;
	# 5 srd = 0;
	
	// odczytujemy stan operacji
	# 5 saddress = 16'h240;
	# 5 srd = 1;
	# 5 srd = 0;
		
    // Finish simulation
    # 1000 $finish;
end
endmodule