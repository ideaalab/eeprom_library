/* 
 * File:   main.c
 * Author: Martin
 *
 * Created on 4 de febrero de 2020, 16:32
 * 
 * Explicacion de funcionamiento: http://www.todopic.com.ar/foros/index.php?topic=49942.0
 */

/*
 * Si se abre como proyecto de MPLAB X, se puede elegir en un desplegable las
 * dos opciones de compilacion: USE_12F o USE_16F
 * Si en cambio se abre de otra manera, descomentar una de las dos lineas
 * de abajo para que compile para uno u otro ejemplo
 */

//#define USE_12F
//#define USE_16F

//escoger que memoria queremos probar
#define TEST_256
//#define TEST_1024

#if defined(USE_12F)
#include <12F1840.h>
#elif defined(USE_16F)
#include <16F1825.h>
#endif

#use delay(clock=32M)    //clock de 32Mhz

#FUSES INTRC_IO, NOWDT, PUT, NOMCLR, NOPROTECT, NOCPD, BROWNOUT, NOCLKOUT, NOIESO, NOFCMEN, NOWRT, PLL_SW, NOSTVREN, BORV25, NODEBUG, NOLVP


/* PUERTOS */
#if defined(USE_12F)
#byte PORTA	= getenv("SFR:PORTA");

#use standard_io(a)				//se accede al puerto a como memoria

#define EEPROM_SCL	PIN_A1		//O
#define EEPROM_SDA	PIN_A2		//O
#define P_TX		PIN_A5		//O

//Bits			    543210
#define TRIS_A	0b00011001		//define cuales son entradas y cuales salidas
#define WPU_A	0b00000000		//define los weak pull up

#elif defined(USE_16F)
#byte PORTA	= getenv("SFR:PORTA");
#byte PORTC	= getenv("SFR:PORTC");

#use standard_io(a)				//se accede al puerto a como memoria
#use standard_io(c)				//se accede al puerto c como memoria

/* PORT A */
#define P_TX		PIN_A0		//O
#define LED			PIN_A1

//Bits				    543210
#define TRIS_A		0b00111100	//define cuales son entradas y cuales salidas
#define WPU_A		0b00000000	//define los weak pull up

/* PORT C */
#define EEPROM_SCL	PIN_C0		//O
#define EEPROM_SDA	PIN_C1		//O

//Bits				    543210
#define TRIS_C		0b00000000	//define cuales son entradas y cuales salidas
#define WPU_C		0b00000000	//define los weak pull up
#endif

#use rs232(baud=115200, xmit=P_TX, DISABLE_INTS, ERRORS)

/* DEFINES */
#define EEPROM_I2C_ADDR_0
#define SAMPLES		16

//unidades en uS
#define MICROSECONDS	1
#define MILISECONDS		(1000UL * MICROSECONDS)
#define SECONDS			(1000UL * MILISECONDS)

/* MACROS */
#define NOW				(get_timer1() + (cont * 0xFFFF))

#if defined(TEST_256)
#include "../24256_mod.c"
#elif defined(TEST_1024)
#include "../241025_mod.c"
#endif

/* VARIABLES */
int buffer[EEPROM_PAGE_SIZE];
long pos = 0;
int32 cont, start, end, total;
int32 tiempo[SAMPLES];	//tiempo, en milisegundos

#int_TIMER1
void Timer1_isr(void){
	cont++;
}

/* PROTOTIPOS */
void old_byte_write(void);
void new_byte_write(void);
void write_pages(void);
void sequential_read(void);
void write_full_eeprom(void);
void read_and_print(void);
void eeprom_full_erase(void);
void printTime(int32 time);
	
void main(void){
	setup_oscillator(OSC_8MHZ|OSC_PLL_ON);	//configura oscilador interno
	setup_wdt(WDT_OFF);						//configura wdt
	setup_timer_0(T0_INTERNAL|T0_DIV_1);	//configura timer0
	setup_timer_1(T1_DIV_BY_8|T1_INTERNAL);	//configura timer1 (1 tick cada 1uS)
	setup_timer_2(T2_DISABLED,255,1);		//configura timer2
	setup_dac(DAC_OFF);						//configura DAC
	setup_adc_ports(NO_ANALOGS);			//configura ADC
	setup_adc(ADC_OFF);						//configura ADC
	setup_ccp1(CCP_OFF);					//configura CCP1
	setup_spi(SPI_DISABLED);				//configura SPI
	setup_vref(VREF_OFF);					//configura VREF			
	setup_adc_ports(NO_ANALOGS);			//configura ADC
	
	set_tris_a(TRIS_A);						//configura pines I/O
	port_a_pullups(WPU_A);					//configura pull ups
	
#if defined(USE_12F)
	setup_comparator(NC_NC);				//configura comparador
	
#elif defined(USE_16F)
	setup_timer_4(T4_DISABLED,255,1);		//configura timer4
	setup_timer_6(T6_DISABLED,255,1);		//configura timer6
	setup_ccp2(CCP_OFF);					//configura CCP2
	setup_ccp3(CCP_OFF);					//configura CCP3
	setup_comparator(NC_NC_NC_NC);			//configura comparador
	setup_dsm(DSM_DISABLED);				//configura DSM
	
	set_tris_c(TRIS_C);						//configura pines I/O
	port_c_pullups(WPU_C);					//configura pull ups
#endif
	
	enable_interrupts(INT_TIMER1);
	enable_interrupts(GLOBAL);
	
	delay_ms(100);
	printf("\r\n- Start -\r\n");
	
	init_ext_eeprom(EXT_EEPROM_400KHZ);		//inicializa eeprom externa a 400Khz
	/* ---------------------------------------------------------------------- */

#if defined(EEPROM_1024K)
	int prev = read_ext_eeprom(0, 0) + 1;
#else
	int prev = read_ext_eeprom(0) + 1;
#endif
	
	//cargamos buffer
	for(int x = 0; x < EEPROM_PAGE_SIZE; x++){
		buffer[x] = prev++;
	}
	
	read_and_print();
	old_byte_write();
	new_byte_write();
	write_pages();
	sequential_read();
	write_full_eeprom();
	eeprom_full_erase();
	
	printf("\r\n- TEST END -\r\n");
	
	do{}while(true);
}

void old_byte_write(void){
	printf("\r\n- %u BYTES OLD WRITE -\r\n", SAMPLES);
	
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = NOW;
#if defined(EEPROM_1024K)
		write_ext_eeprom_old(EXT_EEPROM_BANK_0, x, x);
#else
		write_ext_eeprom_old(0, x);
#endif
		end = NOW;
		tiempo[x] = end - start;
	}
	
	total = NOW;
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Byte %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
}

void new_byte_write(void){
	printf("\r\n- %u BYTES NEW WRITE -\r\n", SAMPLES);
	
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = NOW;
#if defined(EEPROM_1024K)
		write_ext_eeprom(EXT_EEPROM_BANK_0, x, x);
#else
		write_ext_eeprom(0, x);
#endif
		end = NOW;
		tiempo[x] = end - start;
	}
	
	total = NOW;
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Byte %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
}

void write_pages(void){
	printf("\r\n- %u PAGES WRITE (%u bytes/page) -\r\n", SAMPLES, EEPROM_PAGE_SIZE);

	pos = 0;
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = NOW;
#if defined(EEPROM_1024K)
		write_block_ext_eeprom(EXT_EEPROM_BANK_0, 0, EEPROM_PAGE_SIZE, buffer);
#else
		write_block_ext_eeprom(0, EEPROM_PAGE_SIZE, buffer);
#endif
		end = NOW;
		tiempo[x] = end - start;
		pos = pos + EEPROM_PAGE_SIZE;
	}

	total = NOW;
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Page %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
}

void sequential_read(void){
	printf("\r\n- SEQUENTIAL READ %u bytes -\r\n", EEPROM_PAGE_SIZE);

	pos = 0;
	cont = 0;
	set_timer1(0);
	
#if defined(EEPROM_1024K)
	read_block_ext_eeprom(EXT_EEPROM_BANK_0, 0, EEPROM_PAGE_SIZE, buffer);
#else
	read_block_ext_eeprom(0, EEPROM_PAGE_SIZE, buffer);
#endif
	
	total = NOW;
	
	for(int x = 0; x < EEPROM_PAGE_SIZE; x++){
		if((x%8 == 0) && (x != 0)){
			printf("\r\n");
		}
		printf("%02X ", buffer[x]);
	}
	printf("\r\n");
	
	printTime(total);
}

void write_full_eeprom(void){
	printf("\r\n- FULL EEPROM WRITE -\r\n");

	pos = 0;
	cont = 0;
	set_timer1(0);
	
	for(long x = 0; x < EEPROM_PAGES; x++){
#if defined(EEPROM_1024K)
		write_block_ext_eeprom(EXT_EEPROM_BANK_0, pos, EEPROM_PAGE_SIZE, buffer);
		write_block_ext_eeprom(EXT_EEPROM_BANK_1, pos, EEPROM_PAGE_SIZE, buffer);
#else
		write_block_ext_eeprom(pos, EEPROM_PAGE_SIZE, buffer);
#endif
		pos = pos + EEPROM_PAGE_SIZE;
	}

	total = NOW;
	
	printTime(total);
}

void read_and_print(void){
	printf("\r\n- PRINT TO SERIAL -");
#if defined(EEPROM_1024K)
	print_ext_eeprom(EXT_EEPROM_BANK_0, 0, EEPROM_PAGE_SIZE);
#else
	print_ext_eeprom(0, EEPROM_PAGE_SIZE);
#endif
}

void eeprom_full_erase(void){
	printf("\r\n- EEPROM FULL ERASE -\r\n");

	pos = 0;
	cont = 0;
	set_timer1(0);
	
	erase_ext_eeprom();
	total = NOW;
	
	printTime(total);
}

void printTime(int32 time){
long sec, ms, us;

	sec = time / SECONDS;
	ms = (time % SECONDS) / MILISECONDS;
	us = (time % SECONDS) % MILISECONDS;
	
	printf("TOTAL: %Lu Sec %Lu mS %Lu uS\r\n", sec, ms, us);
	delay_ms(5);
}