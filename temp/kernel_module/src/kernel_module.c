#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/sysfs.h>
#include <asm/errno.h>
#include <asm/io.h>


MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patryk Kosiński");
MODULE_DESCRIPTION("Kernel module");
MODULE_VERSION("0.01");


#define SYSFSDRV_BUFFER_LEN PAGE_SIZE

// Maksymalna wielkość bufora

#define MAX_BUFFER_SIZE         128

// Podstawowe makra

#define SYKT_SIZE (0x8000)
#define SYKT_BASE_ADDR (0x00100000)
#define SYKT_EXIT_OFFSET (0x0000)
#define SYKT_EXIT_CODE (0x7F)
#define SYKT_EXIT           (0x3333)

#define MEM_SIZE                16
#define FINISHER_FAIL           (0x3333)
#define RIGHT					1
#define WRONG					0			

// Adresy rejestrów

#define SYKT_REGISTER_A1_READ   (0x0220)
#define SYKT_REGISTER_A2_READ   (0x0228)
#define SYKT_REGISTER_W_WRITE   (0x0230)
#define SYKT_REGISTER_L_WRITE   (0x0238)
#define SYKT_REGISTER_B_WRITE   (0x0240)

// Deklaracja wskaźników wskazujących na odpowiedni obszar w pamięci

void __iomem *baseptr_exit;
void __iomem *baseptr_a1;
void __iomem *baseptr_a2;
void __iomem *baseptr_w;
void __iomem *baseptr_l;
void __iomem *baseptr_b;

// Definicja struktury kobject o nazwie sykt

static struct kobject *sykt;

// Deklaracja zmiennej output, która ma służyć do zapisu na bufor w przypadku uzyskania wyniku W, L lub B

char output[SYSFSDRV_BUFFER_LEN];

// Obsługa wyjątków i wypisywanie komunikatów o błędach konwersji, przepełnieniu i wielkości zadanej liczby w zależności od typu błędu lub flagi

static void exception(int error, long number,int flag){
			
		if (error == 1){
			printk(KERN_WARNING "Blad: kstrtol\n");
		}
		if (error == -ERANGE){
			if (flag == 1) {
				printk(KERN_ERR "Zadana liczba jest za duża!\n");
			}
			else if (flag == 0){
			printk(KERN_ERR "Wystąpiło przepełnienie!Zadana liczba jest za duża!\n");
			}
		}
		if (error == -EINVAL){
			printk(KERN_ERR "Wystąpił błąd konwersji!\n");
		}
	
}

// Odczyt argumentu a1 i zapis na odpowiednie miejsce w pamięci

static ssize_t kpda1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){
		long number; // Zmienna pomocnicza, do której będzie zapisywana wartość bufora 
		int err = kstrtol(buffer, 16, &number); // Wpisywanie wartości z bufora buffer do zmiennej number w zapisie HEX
		exception(err,number,RIGHT); // Wywołanie funkcji, która służy do sprawdzania wyjątków
		writel(number, baseptr_a1); // Zapis pod adres wskazywany przez wskaźnik baseptr_a1
        int n = count > SYSFSDRV_BUFFER_LEN ? SYSFSDRV_BUFFER_LEN : count; // Sprawdzenie, która z wartości jest większa, rozmiar użyteczny bufora count czy jego rozmiar maksymalny; wartość zwracana jest przypisywana do n
		return n;
}

// Odczyt argumentu a2 i zapis na odpowiednie miejsce w pamięci

static ssize_t kpda2_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){
		long number; // Zmienna pomocnicza, do której będzie zapisywana wartość bufora
		int err = kstrtol(buffer, 16, &number); // Wpisywanie wartości z bufora buffer do zmiennej number w zapisie HEX
		exception(err,number,RIGHT); // Wywołanie funkcji, która służy do sprawdzania wyjątków
		writel(number, baseptr_a2); // Zapis pod adres wskazywany przez wskaźnik baseptr_a1
		int n = count > SYSFSDRV_BUFFER_LEN ? SYSFSDRV_BUFFER_LEN : count; // Sprawdzenie, która z wartości jest większa, rozmiar użyteczny bufora count czy jego rozmiar maksymalny; wartość zwracana jest przypisywana do n
		return n;
}

// Odczyt wyniku z modułu

static ssize_t kpdw_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{   
    long digit; // Zmienna pomocnicza do sprawdzania wyjątków
	int i = readl(baseptr_w); // Zmienna pomocnicza, do której będzie zapisywany odczyt wartości z pod adresu wskazywanego przez wskaźnik baseptr_w
	sprintf(output, "%X\n", i); // Przypisanie wartości ze zmiennej pomocniczej "i" na "output" w zapisie HEX
	int err = kstrtol(output, 16, &digit); // Wpisywanie wartości z "output" do zmiennej digit w zapisie HEX
	exception(err,digit,WRONG); // Wywołanie funkcji, która służy do sprawdzania wyjątków
	memcpy((void*)buffer, output, strnlen(output, SYSFSDRV_BUFFER_LEN)+1); // Kopiowanie danych z output na bufor buffer
	// Funkcja strnlen zwraca długość łańcucha znaków w zmiennej output, ale długość ta nigdy nie przekracza maksymalnej wielkości bufora
	// Dodaję 1 aby skopiować również null na końcu łańcucha
    return strnlen(output, SYSFSDRV_BUFFER_LEN);
}

// Odczyt licznika z modułu

static ssize_t kpdl_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{   
	int i = readl(baseptr_l); // Zmienna pomocnicza, do której będzie zapisywany odczyt wartości z pod adresu wskazywanego przez wskaźnik baseptr_l
	sprintf(output, "%X\n", i); // Przypisanie wartości ze zmiennej pomocniczej "i" na "output" w zapisie HEX
	memcpy((void*)buffer, output, strnlen(output, SYSFSDRV_BUFFER_LEN)+1); // Kopiowanie danych z output na bufor buffer
	// Funkcja strnlen zwraca długość łańcucha znaków w zmiennej output, ale długość ta nigdy nie przekracza maksymalnej wielkości bufora
	// Dodaję 1 aby skopiować również null na końcu łańcucha
    return strnlen(output, SYSFSDRV_BUFFER_LEN);
}

// Odczyt statusu z modułu

static ssize_t kpdb_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{   
	int i = readl(baseptr_b); // Zmienna pomocnicza, do której będzie zapisywany odczyt wartości z pod adresu wskazywanego przez wskaźnik baseptr_b
	sprintf(output, "%X\n", i); // Przypisanie wartości ze zmiennej pomocniczej "i" na "output" w zapisie HEX
	memcpy((void*)buffer, output, strnlen(output, SYSFSDRV_BUFFER_LEN)+1); // Kopiowanie danych z output na bufor buffer
	// Funkcja strnlen zwraca długość łańcucha znaków w zmiennej output, ale długość ta nigdy nie przekracza maksymalnej wielkości bufora
	// Dodaję 1 aby skopiować również null na końcu łańcucha
    return strnlen(output, SYSFSDRV_BUFFER_LEN);
}


// Deklaracja atrybutów tylko do odczytu (RO) i tylko do zapisu (WO)

static struct kobj_attribute a1_attr = __ATTR_WO(kpda1);
static struct kobj_attribute a2_attr = __ATTR_WO(kpda2);
static struct kobj_attribute w_attr = __ATTR_RO(kpdw);
static struct kobj_attribute l_attr = __ATTR_RO(kpdl);
static struct kobj_attribute b_attr = __ATTR_RO(kpdb);

// Inicjalizacja modułu

int my_init_module(void){
    printk(KERN_INFO "Inicjalizacja modulu\n");
	
	// Zagospodarowanie miejsca w pamięci
	
    baseptr_exit = ioremap(SYKT_BASE_ADDR + SYKT_EXIT_OFFSET, SYKT_SIZE);
    baseptr_a1 = ioremap(SYKT_BASE_ADDR + SYKT_REGISTER_A1_READ, SYKT_SIZE);
    baseptr_a2 = ioremap(SYKT_BASE_ADDR + SYKT_REGISTER_A2_READ, SYKT_SIZE);
    baseptr_w = ioremap(SYKT_BASE_ADDR + SYKT_REGISTER_W_WRITE, SYKT_SIZE);
    baseptr_l = ioremap(SYKT_BASE_ADDR + SYKT_REGISTER_L_WRITE, SYKT_SIZE);
    baseptr_b = ioremap(SYKT_BASE_ADDR + SYKT_REGISTER_B_WRITE, SYKT_SIZE);
	
	// Tworzenie obiektu typu kobject
	
    sykt = kobject_create_and_add("sykt", kernel_kobj);
	
	// Tworzenie plików sysfs dla poszczególnych atrybutów
	
    int err_a1 = sysfs_create_file(sykt, &a1_attr.attr);
    int err_a2 = sysfs_create_file(sykt, &a2_attr.attr);
    int err_w = sysfs_create_file(sykt, &w_attr.attr);
    int err_l = sysfs_create_file(sykt, &l_attr.attr);
    int err_b = sysfs_create_file(sykt, &b_attr.attr);

	
	// Komunikaty o błędach przy tworzeniu pliku dla danego atrybutu
	
	if(err_a1){
    	printk(KERN_INFO"Cannot create sysfs_a1 file......\n");
		return 1;
	}

	if(err_a2){
    	printk(KERN_INFO"Cannot create sysfs_a2 file......\n");
		sysfs_remove_file(kernel_kobj, &a1_attr.attr);
		return 1;
	}

	if(err_b){
    	printk(KERN_INFO"Cannot create sysfs file_b......\n");
		sysfs_remove_file(kernel_kobj, &a1_attr.attr);
		sysfs_remove_file(kernel_kobj, &a2_attr.attr);
		return 1;
	}

	if(err_w){
    	printk(KERN_INFO"Cannot create sysfs file_w......\n");
		sysfs_remove_file(kernel_kobj, &a1_attr.attr);
		sysfs_remove_file(kernel_kobj, &a2_attr.attr);
		sysfs_remove_file(kernel_kobj, &b_attr.attr);
		return 1;
	}
	if(err_l){
    	printk(KERN_INFO"Cannot create sysfs file_l......\n");
		sysfs_remove_file(kernel_kobj, &a1_attr.attr);
		sysfs_remove_file(kernel_kobj, &a2_attr.attr);
		sysfs_remove_file(kernel_kobj, &b_attr.attr);
		sysfs_remove_file(kernel_kobj, &w_attr.attr);
		return 1;
	}
	
	

    return 0;
}

void my_cleanup_module(void){
	printk(KERN_INFO "Cleanup my sykt module.\n"); 
	writel(SYKT_EXIT | ((SYKT_EXIT_CODE)<<16), baseptr_exit); 
	
	// Zwalnianie obiektu sykt. Według dokumentacji, taka metoda to jedyna bezpieczna opcja
	
	kobject_put(sykt);
	
	// Zwalnianie pamięci
	
	iounmap(baseptr_exit);
    iounmap(baseptr_a1);
    iounmap(baseptr_a2);
    iounmap(baseptr_w);
    iounmap(baseptr_l);
    iounmap(baseptr_b);	
	
	
}
module_init(my_init_module)
module_exit(my_cleanup_module)

