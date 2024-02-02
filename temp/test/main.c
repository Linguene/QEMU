#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define MAX_BUFFER 1024
#define SYSFS_FILE_A1 "/sys/kernel/sykt/kpda1"
#define SYSFS_FILE_A2 "/sys/kernel/sykt/kpda2"
#define SYSFS_FILE_W "/sys/kernel/sykt/kpdw"
#define SYSFS_FILE_L "/sys/kernel/sykt/kpdl"
#define SYSFS_FILE_B "/sys/kernel/sykt/kpdb"

#define RIGHT	1 
#define WRONG	0 
#define MAX_INPUT_SIZE 6 

// Funkcja odczytująca z pliku

unsigned int read_int(const char *name)
{
    FILE *f;
    unsigned long read;
    unsigned long value;
    char *number;
    int error;
    f = fopen(name, "r");

    // obsługa wyjątków
    if (f == NULL) {
        fprintf(stderr, "Nieudana próba otwarcia pliku '%s': ", name);
        perror("");
        exit(EXIT_FAILURE);
    }

    read = fscanf(f, "%lx", &value);

    if (read != 1) {
        printf("%x", errno);
        printf("Nieudany odczyt z pliku '%s'\n", name);
        exit(EXIT_FAILURE);
    }

    fclose(f);

    return value;
}

// Funkcja zapisująca do pliku

int write_int(const char *name, unsigned long value)
{
    FILE *f;
    int ret;
    char buffer[MAX_BUFFER];

    f = fopen(name, "w");
    int error = snprintf(buffer, MAX_BUFFER, "%lx", value);
	
	// Obsługa wyjątków
    if (f == NULL) {
        fprintf(stderr, "Nieudana próba otwarcia pliku '%s': ", name);
        perror("");
        exit(EXIT_FAILURE);
    }
    else if (error < 0) {
        printf("Nieudany zapis do pliku '%s'\n", name);
        fclose(f);
        exit(EXIT_FAILURE);
    }
    else if (error == -ERANGE) {
        fprintf(stderr, "Zadana liczba jest za duża!\n");
    }
    else if (error == -EINVAL) {
        fprintf(stderr, "Wystąpił błąd konwersji!\n");
    }
    fwrite(buffer, strlen(buffer), 1, f);

    fclose(f);
    return 0;
}

// Struktura listy do auto_multiplicatora
struct Lista {
    unsigned int value1;
    unsigned int value2;
    unsigned int value3;
};

// Funkcja, która ma za zadanie dostarczyć prawdziwy wynik operacji mnożenia, wyniku licznika i oczekiwanego statusu końcowego operacji
static int auto_multiplicator(unsigned int a1, unsigned int a2, struct Lista *lista)
{
    unsigned int result = a1 * a2;
    unsigned int counter = __builtin_popcount(result); // Liczba '1' w notacji binarnej liczby
    size_t status;

    if (((result / a2) != a1) && a2 != 0 && a1 != 0) {
        result = 0;
        counter = 0;
        status = 2; //status dla niepoprawnie wykonanej operacji/przepełnienia
    }
    else if (a1 == 0 || a2 == 0) {
        result = 0;
        counter = 0;
        status = 3; //status dla poprawnie wykonanej operacji - mnożenie przez zero
    }
    else {
        status = 3; //status dla poprawnie wykonanej operacji w innym wypadku niż mnożenie przez zero
    }

    lista->value1 = result;
    lista->value2 = counter;
    lista->value3 = status;

    return 0;
}

void multiply(unsigned int a1, unsigned int a2, unsigned long* w, unsigned long* l, unsigned long* b)
{
    write_int(SYSFS_FILE_A1, a1);
    write_int(SYSFS_FILE_A2, a2);
    
    //pętla wykonująca się do momentu aż status zmieni się na 2 lub 3, co świadczy o zakończeniu operacji
	
    while (read_int(SYSFS_FILE_B) < 2) {
    }
    *w = read_int(SYSFS_FILE_W);
    *l = read_int(SYSFS_FILE_L);
    *b = read_int(SYSFS_FILE_B);
}

static int test(size_t num_of_operations, ...)
{
    size_t i;
    unsigned long w, l, b;
    unsigned int a1;
    unsigned int a2;
    struct Lista lista;
    printf("Rozpoczęcie testowania\n");

    va_list args;
    va_start(args, num_of_operations);
    for (int i = 0; i < num_of_operations; i++) {
        a1 = va_arg(args, unsigned int);
        a2 = va_arg(args, unsigned int);
        multiply(a1, a2, &w, &l, &b);
        auto_multiplicator(a1, a2, &lista);
        if (lista.value1 == w && lista.value2 == l &&lista.value3 == b) {
            printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Success!\n", a1, a2, w, l, b); // Warunek sprawdzający sukces
        }
        else {
			if ((a1 > 0xFFFFFF && a2 <= 0xFFFFFF) && w == 0x404 && b == 2){
				printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Too big A1 value !\n", a1, a2, w, l, b);
				// Warunek sprawdzający, czy w A1 nie została zapisana zbyt duża wartość
			}
			else if ((a2 > 0xFFFFFF && a1 <= 0xFFFFFF) && w == 0x404 && b == 2) {
				printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Too big A2 value !\n", a1, a2, w, l, b);
				// Warunek sprawdzający, czy w A2 nie została zapisana zbyt duża wartość
			}
			else if ((a1 > 0xFFFFFF && a2 > 0xFFFFFF) && w == 0x404 && b == 2){
				printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Too big A1 and A2 values !\n", a1, a2, w, l, b);
				// Warunek sprawdzający, czy w A1 i w A2 nie została zapisana zbyt duża wartość
			}
			else if ((a1 <= 0xFFFFFF && a2 <= 0xFFFFFF) && w == 0x404 && b == 2){
				printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Overflow!\n", a1, a2, w, l, b);
				// Warunek sprawdzający, czy nie wystąpił overflow
				// Chodzi o to, że jeśli obie dane wejściowe mają poprawną wielkość, a status wraz jest równy 2, to mamy do czynienia z przepełnieniem
			}
			else{
				printf("\nA1: '%lx', A2: '%lx', W: '%x', L: '%lx', B: '%x'		Failed!\n", a1, a2, w, l, b);
				// Jeśli błąd nie jest powodowany przez powyższe warunki, to wypisywany jest komunikat Failed!
			}
			
        }
    }
    va_end(args);

    return 0;
}
// Struktura przechowująca dane do testowania
unsigned long test_data[][2] = {
    {0x000005, 0x000002},
    {0x000000, 0x000001},
    {0x000002, 0x000003},
    {0xFFFFFF, 0xFFFFFF},
	{0xFFFFFF1, 0x000005},
	{0x000005, 0xFFFFFF1},
    {0x00000A, 0x00000E},
	{0x0001C2, 0x0000C8},
	{0xFFFFFF1, 0xFFFFFF2}
};

int main(void)
{
    unsigned int a1, a2, w, l, b;

    size_t num_of_operations;
	// Obliczany numer operacji
    num_of_operations = sizeof(test_data) / sizeof(test_data[0]);
	// Przekazywanie danych do testowania
    test(num_of_operations,
        test_data[0][0], test_data[0][1],
        test_data[1][0], test_data[1][1],
        test_data[2][0], test_data[2][1],
        test_data[3][0], test_data[3][1],
        test_data[4][0], test_data[4][1],
        test_data[5][0], test_data[5][1],
        test_data[6][0], test_data[6][1],
		test_data[7][0], test_data[7][1],
		test_data[8][0], test_data[8][1]
    );

    puts("\n");

    return 0;
}