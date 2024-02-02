/* verilator lint_off UNUSED */
/* verilator lint_off MULTIDRIVEN */
/* verilator lint_off UNDRIVEN */
/* verilator lint_off COMBDLY */
/* verilator lint_off WIDTH */
/* verilator lint_off BLKSEQ */
/* verilator lint_off BLKANDNBLK */
/* verilator lint_off CASEINCOMPLETE */
/* verilator lint_off UNOPTFLAT */
/* verilator lint_off UNSIGNED */
/* verilator lint_off CMPCONST */

// Zdefiniowanie modułu gpioemu

module gpioemu(n_reset, saddress[15:0], srd, swr, sdata_in[31:0], sdata_out[31:0], gpio_in[31:0], gpio_latch, gpio_out[31:0], clk, gpio_in_s_insp[31:0]);
	
	// Definicje sygnałów wejściowych
	
	input 				n_reset; 
	input  [15:0] 		saddress; 
	input 				srd; 
	input 				swr; 
	input  [31:0] 		sdata_in; 
	input  [31:0] 		gpio_in; 
	input				gpio_latch; 
	input 				clk;
	
	// Definicje sygnałów wyjściowych
	
	output [31:0] 		sdata_out; 
	output [31:0] 		gpio_out; 
    output [31:0]       gpio_in_s_insp;
	
	// Deklaracje rejestrów w module 
	
    reg    [31:0]       gpio_out_s;
    reg    [31:0]       gpio_in_s;
    reg    [31:0]       sdata_out_s;

    reg    [23:0]       A1; // rejestr przechowujący daną wejściową pierwszą
    reg    [23:0]       A2; // rejestr przechowujący daną wejściową drugą
    reg    [31:0]       W;  // rejestr przechowyujący wynik
    reg    [31:0]       L;  // rejestr przechowujący licznik 
    reg    [31:0]       B;  // rejestr przechowujący status
	
	// Status 0 oznacza, że moduł nie rozpoczął swojej pracy
	// Status 1 oznacza, że moduł jest aktywny, a konkretnie, że rejestr A2 otrzymał nową wartość
	// Status 2 oznacza, że wystąpiło przepełnienie (overflow), a co za tym idzie, wynik nie jest poprawny
	// Status 3 oznacza, że operacja została zakończona i otrzymaliśmy poprawny wynik
	
	reg	   [47:0]		product; // rejestr zliczający w iteracji wynik
	reg    [47:0]       result; // rejestr, do którego przypisywany jest "product"
	reg    [31:0]       sumcheck; // rejestr, który zlicza ilość jedynek i przypisuje je do licznika
    reg    [31:0] 		A1_control; // rejestr badający występowanie nieprawidłowych wartości
	
	// Deklaracja zmiennych pomocniczych - flaga, integery do iteracji
	
	reg flag;  
	integer i = 0; 
	integer k = 0; 

	// Deklaracja parametrów
	
	parameter max_SIZE_INPUT = 24;
	parameter max_SIZE_OUTPUT = 32;
	parameter total_SIZE = 48;


	// Reset zmiennych przed rozpoczęciem właściwej pracy modułu
	
    always @(negedge n_reset)
	begin
		sdata_out_s <= 0;
		gpio_out_s <= 0;
		W <= 0;
		L <= 0;
		B <= 16'h0000; 
		i <= 0;
		k<= 0;
		flag<=0;
		product <= 0;
		result <=0;
		sumcheck <=0;
	end
	
	// Definicja pracy zatrzasku
	
    always @(posedge gpio_latch) // zatrzask
	begin
		gpio_in_s <= gpio_in;
	end
	
	// Reset rejestrów i integerów po załadowaniu kolejnej wartości do rejestru A2 (czyli po prostu przy kolejnej operacji)
	
	always@(*) begin
		if (flag==1) begin
			W <= 0;
			L <= 0;
			sumcheck<=0;
			product <=0;
			flag<=0;
			i <= 0;
			k <= 0;
		end
	end

	// Zapis danych z szyny wejściowej do rejestrów A1 i A2
	
    always @(posedge swr) // zapis danych
    begin
        if (B==16'h0002 || B==16'h0003 || B==16'h0000) // Odczytywanie z szyny wejściowej nie powinno mieć miejsca, gdy moduł jest aktywny
		begin
			case(saddress)
				16'h220: begin
							A1_control <= sdata_in;
							A1 <= sdata_in;
							B <= 0;
						end
				16'h228: begin
							A2 <= sdata_in;
							flag<=1;
							B <= 1; // Moduł rozpoczyna swoją pracę
						end
			endcase
		end
    end
	
	// Odczyt danych z modułu (W, L, B) i ich zapis 
	
	always @(posedge srd) begin // Odczyt danych z modułu
			case(saddress)
			16'h230: sdata_out_s <=W;
			16'h238: sdata_out_s <=L;
			16'h240: sdata_out_s <=B;
			default: sdata_out_s <=0;
			endcase
		end

	// Definicja obliczania odpowiednich wartości w module
	
	always @(posedge clk)
	begin
		if (B == 1) // Operacja wykonywana wtedy, kiedy moduł jest aktywny
		begin
			product <= 0; // Rejestr "product" na początku przechowuje wartość 0
			sumcheck <=0; // Rejestr "sumcheck" na początku przechowuje wartość 0
			for (i = 0; i < max_SIZE_INPUT; i = i + 1) begin
                if (A2[i] == 1) begin
                    product = product + (A1 << i);
					// Mnożenie "bit po bicie" - jeśli jest znalezione "1" na A2, dodajemy do poprzedniej wartości w rejestrze "product" wartość A1 z przesunięciem (pętla)
                end
				if (i==max_SIZE_INPUT-1)begin
					result = product; // Na końcu pętli, wartość w rejestrze "product" jest przypisywana do rejestru "result"
				end
			end
			for (k = 0; k < total_SIZE; k = k + 1) begin
                     if (result[k] == 1) begin
                         sumcheck = sumcheck + 1; // Rejestr "sumcheck" zlicza "1" w wyniku
					 end
					 else begin
						 sumcheck = sumcheck + 0; // Jeśli nie ma "1" na danej pozycji, wartość w rejestrze "sumcheck" pozostaje taka sama
					 end

			if (A1!=0 && A2!=0) begin 
				if ((sdata_in>24'hFFFFFF && A1_control<24'hFFFFFF) || (A1_control>24'hFFFFFF && sdata_in<24'hFFFFFF)) begin 
					// Kontrola danych wejściowych, aby nie przekraczały maksymalnej wielkości bitów - kiedy tak się dzieje, oczekujemy komunikatu o błędzie
					B <= 16'h0002; // Zakończenie operacji, status świadczy o niepoprawnym wyniku
					W <= 32'h00000404; // Wartość, która oznacza wystąpienie przepełnienia przypisywana do rejestru "W"
					L <= 32'h00000000; 
					i <= 0;
					k <= 0;
					gpio_out_s <= gpio_out_s + 1; // Zliczanie ilości wykonanych operacji
				end
		
				else if(result > 2**max_SIZE_OUTPUT - 1) begin   // Badanie wystąpienia przepełnienia
					
					B <= 16'h0002; // Zakończenie operacji, status świadczy o niepoprawnym wyniku
					W <= 32'h00000404; // Wartość, która oznacza wystąpienie przepełnienia przypisywana do rejestru "W"
					L <= 32'h00000000; 
					i <= 0;
					k <= 0;
					gpio_out_s <= gpio_out_s + 1; // Zliczanie ilości wykonanych operacji
			
				end
				
				
				else begin
					B <= 16'h0003; // Zakończenie operacji, status świadczy o poprawnym wyniku
					i <= 0;
					k <= 0;
					L <=sumcheck; // Do rejestru "L" przypisywany jest zliczona ilość jedynek
					W <= result; // Do rejestru "W" przypisywany jest prawdziwy wynik
					gpio_out_s <= gpio_out_s + 1; // Zliczanie ilości wykonanych operacji
				end
			
			end
			else  // Kiedy przynajmniej jedna z danych jest równa 0, wynik automatycznie wynosi 0, a status jest równy 3
				begin 
					B <= 16'h0003;
					L <=sumcheck;
					W <= product;
					i <= 0;
					k <= 0;
					gpio_out_s <= gpio_out_s + 1; // Zliczanie ilości wykonanych operacji
				end
            end
		end
	end

	assign gpio_out = gpio_out_s[15:0]; // 16 bitów numeru wykonanych operacji wyprowadzanych z modułu na wyprowadzenia GPIO procesora
	assign sdata_out = sdata_out_s; // Wyprowadzenie wyniku na szynę wyjściową
	assign gpio_in_s_insp = gpio_in_s;

endmodule