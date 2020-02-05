/* 
 * File:   main.c
 * Author: Martin
 *
 * Created on 4 de febrero de 2020, 16:32
 */
#include <16F1825.h>
#use delay(clock=32M)    //clock de 32Mhz

#FUSES INTRC_IO, NOWDT, PUT, NOMCLR, NOPROTECT, NOCPD, BROWNOUT, NOCLKOUT, NOIESO, NOFCMEN, NOWRT, PLL_SW, NOSTVREN, BORV25, NODEBUG, NOLVP

/* PUERTOS */
#use standard_io(a)				//se accede al puerto a como memoria
#use standard_io(c)				//se accede al puerto c como memoria

/* PORT A */
#define P_TX		PIN_A0		//O
#define P_A1		PIN_A1		//I
#define P_A2		PIN_A2		//I
#define P_A3		PIN_A3		//I
#define P_A4		PIN_A4		//1
#define P_A5		PIN_A5		//1

//Bits				    543210
#define TRIS_A		0b00111110	//define cuales son entradas y cuales salidas
#define WPU_A		0b00000000	//define los weak pull up

/* PORT C */
#define EEPROM_SCL	PIN_C0		//O
#define EEPROM_SDA	PIN_C1		//O
#define P_C2		PIN_C2		//I
#define P_C3		PIN_C3		//I
#define P_C4		PIN_C4		//I
#define P_C5		PIN_C5		//I

//Bits				    543210
#define TRIS_C		0b00111100	//define cuales son entradas y cuales salidas
#define WPU_C		0b00000000	//define los weak pull up

#use rs232(baud=115200, xmit=P_TX, DISABLE_INTS, ERRORS)

/* DEFINES */
#define EEPROM_I2C_ADDR_0
#define SAMPLES		16

//unidades en uS
#define MICROSECONDS	1
#define MILISECONDS		(1000UL * MICROSECONDS)
#define SECONDS			(1000UL * MILISECONDS)

#include "../241025_mod.c"
//#include "../24256_mod.c"

/* VARIABLES */
int buffer[EEPROM_PAGE_SIZE];
long pos = 0;
int32 cont, start, end, total;
int32 tiempo[SAMPLES];	//tiempo, en milisegundos

#int_TIMER1
void Timer1_isr(void){
	cont++;
}

void printTime(int32 time){
long sec, ms, us;

	sec = time / SECONDS;
	ms = (time % SECONDS) / MILISECONDS;
	us = (time % SECONDS) % MILISECONDS;
	
	printf("TOTAL: %Lu Sec %Lu mS %Lu uS\r\n", sec, ms, us);
}

void main(void){
	setup_oscillator(OSC_8MHZ|OSC_PLL_ON);	//configura oscilador interno
	setup_wdt(WDT_OFF);						//configura wdt
	setup_timer_0(T0_INTERNAL|T0_DIV_1);	//configura timer0
	setup_timer_1(T1_DIV_BY_8|T1_INTERNAL);	//configura timer1
	setup_timer_2(T2_DISABLED,255,1);		//configura timer2
	setup_timer_4(T4_DISABLED,255,1);		//configura timer4
	setup_timer_6(T6_DISABLED,255,1);		//configura timer6
	setup_dac(DAC_OFF);						//configura DAC
	setup_adc_ports(NO_ANALOGS);			//configura ADC
	setup_adc(ADC_OFF);						//configura ADC
	setup_ccp1(CCP_OFF);					//configura CCP1
	setup_ccp2(CCP_OFF);					//configura CCP2
	setup_ccp3(CCP_OFF);					//configura CCP3
	setup_spi(SPI_DISABLED);				//configura SPI
	setup_vref(VREF_OFF);					//configura VREF			
	setup_comparator(NC_NC_NC_NC);			//configura comparador
	setup_dsm(DSM_DISABLED);				//configura DSM
	
	set_tris_a(TRIS_A);						//configura pines I/O
	port_a_pullups(WPU_A);					//configura pull ups
	
	set_tris_c(TRIS_C);						//configura pines I/O
	port_c_pullups(WPU_C);					//configura pull ups
	
	enable_interrupts(INT_TIMER1);
	enable_interrupts(GLOBAL);
	
	init_ext_eeprom(EXT_EEPROM_400KHZ);		//inicializa eeprom externa a 400Khz
	/* ---------------------------------------------------------------------- */
	
	//cargamos buffer
	for(int x = 0; x < EEPROM_PAGE_SIZE; x++){
		buffer[x] = 0xFF;
	}
	
	delay_ms(100);
	printf("\r\n- Start -\r\n");
	
	/* OLD BYTE WRITE ------------------------------------------------------- */
	printf("\r\n- %u BYTES OLD WRITE -\r\n", SAMPLES);
	
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = get_timer1() + (cont * 0xFFFF);
#if defined(EEPROM_1024K)
		write_ext_eeprom_old(EXT_EEPROM_BANK_0, 0, 0xFF);
#else
		write_ext_eeprom_old(0, 0xFF);
#endif
		end = get_timer1() + (cont * 0xFFFF);
		tiempo[x] = end - start;
	}
	
	total = get_timer1() + (cont * 0xFFFF);
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Byte %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
	delay_ms(5);
	/* ---------------------------------------------------------------------- */
	
	/* NEW BYTE WRITE ------------------------------------------------------- */
	printf("\r\n- %u BYTES NEW WRITE -\r\n", SAMPLES);
	
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = get_timer1() + (cont * 0xFFFF);
#if defined(EEPROM_1024K)
		write_ext_eeprom(EXT_EEPROM_BANK_0, 0, 0xFF);
#else
		write_ext_eeprom(0, 0xFF);
#endif
		end = get_timer1() + (cont * 0xFFFF);
		tiempo[x] = end - start;
	}
	
	total = get_timer1() + (cont * 0xFFFF);
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Byte %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
	delay_ms(5);
	/* ---------------------------------------------------------------------- */
	
	/* WRITE PAGES ---------------------------------------------------------- */
	printf("\r\n- %u PAGES WRITE (%u bytes/page) -\r\n", SAMPLES, EEPROM_PAGE_SIZE);

	pos = 0;
	cont = 0;
	set_timer1(0);
	
	for(int x = 0; x < SAMPLES; x++){
		start = get_timer1() + (cont * 0xFFFF);
#if defined(EEPROM_1024K)
		write_block_ext_eeprom(EXT_EEPROM_BANK_0, 0, EEPROM_PAGE_SIZE, buffer);
#else
		write_block_ext_eeprom(0, EEPROM_PAGE_SIZE, buffer);
#endif
		end = get_timer1() + (cont * 0xFFFF);
		tiempo[x] = end - start;
		pos = pos + EEPROM_PAGE_SIZE;
	}

	total = get_timer1() + (cont * 0xFFFF);
	
	for(int x = 0; x < SAMPLES; x++){
		printf("Page %2u: %Lu uS\r\n", x, tiempo[x]);
	}
	
	printTime(total);
	delay_ms(5);
	/* ---------------------------------------------------------------------- */
	
	/* WRITE FULL EEPROM ---------------------------------------------------- */
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

	total = get_timer1() + (cont * 0xFFFF);
	
	printTime(total);
	delay_ms(5);
	/* ---------------------------------------------------------------------- */
	
	do{}while(true);
	
}

