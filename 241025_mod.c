/*******************************************************************************
 * Libreria creada por Martin Andersen para IDEAA Lab
 * Basada en la libreria de CCS info
 ******************************************************************************/

/*******************************************************************************
 * ABOUT
 * 
 * Esta libreria elimina el tiempo de espera DESPUES de las grabaciones.
 * La escritura requiere unos 3-5 mS de tiempo. Aunque no hace falta quedarse
 * esperando hasta que termine.
 ******************************************************************************/

/*******************************************************************************
 * CONFIGIRACION
 * 
 * + Para que la libreria funcione hay que declarar los pines que se van a usar:
 *  Ejemplo:
 *	#define EEPROM_SCL	PIN_C0
 *	#define EEPROM_SDA	PIN_C1
 * 
 * + La memoria puede tener 4 direcciones I2C posibles. Se puede configurar
 *	declarando un define EEPROM_I2C_ADDR_0 / EEPROM_I2C_ADDR_1 /
 *	EEPROM_I2C_ADDR_2 / EEPROM_I2C_ADDR_3
 *	Por defecto, si no se declara se usa la direccion 0.
 *	Ejemplo:
 *	#define EEPROM_I2C_ADDR_1
 ******************************************************************************/

/*******************************************************************************
 * FUNCIONES
 * 
 * + init_ext_eeprom()
 *		llamar antes de usar la libreria
 * 
 * + write_ext_eeprom(short bsb, long address, int data)
 *		escribe en la memoria:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-address: posicion de memoria (0 - 65535)
 *		-data: valor a escribir (0 - 255)
 * 
 * + read_ext_eeprom(short bsb, long address)
 *		lee de la memoria y devuelve un INT con el valor leido:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-address: posicion de memoria (0 - 65535)
 * 
  * + read_block_ext_eeprom(short bsb, long start, long len, int* data)
 *		lee un bloque memoria a un array:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-start: posicion de comienzo de memoria (0 - 65535)
 *		-len: cantidad de datos a leer (1 - 65535)
 *		-*data: puntero del array donde vamos a guardar los datos
 *			CUIDADO, la funcion no comprueba el tamaño del array. Tenemos
 *			que pasarle un array del al menos el tamaño "len" para que no
 *			sobreescriba variables adyacentes.
 *			Ejemplo:
 *			int valores[64];
 *			read_block_ext_eeprom(0, 0, 64, valores);
 ******************************************************************************/

/*******************************************************************************
 * POSIBLES MEJORAS
 * + Se puede implementar grabacion y lectura por bancos completos
 ******************************************************************************/

/*
 * Control byte:
 * 0: R/W (1=R/0=W)
 * 1: Address 0
 * 2: Address 1
 * 3: Block select bit
 * 4-7: Control code (0b1010)
 */

#ifndef EEPROM_SDA
#ERROR Hay que declarar el pin EEPROM_SDA
#endif

#ifndef EEPROM_SCL
#ERROR Hay que declarar el pin EEPROM_SCL
#endif

#define EXT_EEPROM_ACK		0
#define EXT_EEPROM_NO_ACK	1

//usadas con i2c_read()
//cuando queremos terminar la lectura usamos EXT_EEPROM_RESPONSE_NO_ACK
//si queremos seguir leyendo usamos EXT_EEPROM_RESPONSE_ACK
#define EXT_EEPROM_RESPONSE_ACK		1
#define EXT_EEPROM_RESPONSE_NO_ACK	0

#if defined(EEPROM_I2C_ADDR_3)
#define CONTROL_BYTE_WRITE	0b10100110
#define CONTROL_BYTE_READ	0b10100111
#elif defined(EEPROM_I2C_ADDR_2)
#define CONTROL_BYTE_WRITE	0b10100100
#define CONTROL_BYTE_READ	0b10100101
#elif defined(EEPROM_I2C_ADDR_1)
#define CONTROL_BYTE_WRITE	0b10100010
#define CONTROL_BYTE_READ	0b10100011
#else	//EEPROM_I2C_ADDR_0
#define CONTROL_BYTE_WRITE	0b10100000
#define CONTROL_BYTE_READ	0b10100001
#endif

//hay que forzar I2C por software, sino no funciona ¿?
#use i2c(FORCE_SW, master, sda=EEPROM_SDA, scl=EEPROM_SCL)
//#use i2c(master, sda=EEPROM_SDA, scl=EEPROM_SCL)

#define EEPROM_ADDRESS long int
#define EEPROM_PAGE_SIZE	128
#define EEPROM_SIZE			65536		//en realidad el tamaño de la memoria es del doble, este es el tamaño de uno de los 2 bloques

#define EXT_EEPROM_BANK_0	0
#define EXT_EEPROM_BANK_1	1

/*
 * Incializa la EEPROM externa
 */
void init_ext_eeprom(void){
   output_float(EEPROM_SCL);
   output_float(EEPROM_SDA);
}

/*
 * Escribe en la EEPROM externa
 * bsb es uno de los dos bloques disponibles (block selection bit)
 * address es la direccion de memoria a escribir
 * data es el byte que escribiremos en la direccion
 */
/*void write_ext_eeprom(short bsb, long address, int data){
short resp;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	i2c_start();			//start condition
	i2c_write(ControlByteW);	//control byte (write)
	i2c_write(address>>8);	//address high
	i2c_write(address);		//address low
	i2c_write(data);		//data byte
	i2c_stop();				//stop condition
	
	do{
		i2c_start();
		resp = i2c_write(ControlByteW);
	}while(resp == EXT_EEPROM_NO_ACK);
	
	i2c_stop();
}*/

/*
 * Escribe en la EEPROM externa, pero NO espera despues de escribir
 * En cambio, espera ANTES de escribir. De esta forma podemos volver al
 * programa justo despues de escribir, sin esperar. Si fuesemos a escribir
 * otro valor antes de que la escritura anterior haya acabado, entonces
 * esperamos a que termine antes de continuar.
 * bsb es uno de los dos bloques disponibles (block selection bit)
 * address es la direccion de memoria a escribir
 * data es el byte que escribiremos en la direccion
 */
void write_ext_eeprom(short bsb, long address, int data){
short resp;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	//wait for the memory to be ready
	do{
		i2c_start();
		resp = i2c_write(ControlByteW);
	}while(resp == EXT_EEPROM_NO_ACK);
	
	i2c_write(address>>8);	//address high
	i2c_write(address);		//address low
	i2c_write(data);		//data byte
	i2c_stop();				//stop condition
}

/*
 * Escribe varios datos el mismo tiempo en la EEPROM
 * bsb es uno de los dos bloques disponibles
 * address es la posicion de inicio de grabacion
 * data es un puntero a los datos a escribir
 * len es la cantidad de bytes a escribir
 */
void write_ext_eeprom(short bsb, long start, int len, int* data){
#warning "implementar"
}

/*
 * Lee de la EEPROM externa
 * Lee un byte del bloque y direccion que le pasamos
 * Comprueba que haya terminado cualquier grabacion antes de leer.
 */
int read_ext_eeprom(short bsb, long address){
short resp;
int data;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
int ControlByteR = CONTROL_BYTE_READ | ((int)bsb<<3);

   //esperamos que la memoria este lista
	do{
		i2c_start();
		resp = i2c_write(ControlByteW);
	}while(resp == EXT_EEPROM_NO_ACK);
	
	i2c_write(address>>8);		//address high
	i2c_write(address);			//address low

	i2c_start();				//start condition
	i2c_write(ControlByteR);	//control byte (read)
	data = i2c_read(EXT_EEPROM_RESPONSE_NO_ACK);	//read byte
	i2c_stop();					//stop condition

	return(data);
}

/*
 * Lee varios datos el mismo tiempo de la EEPROM
 * address es la posicion de inicio de lectura
 * data es un puntero a la variable donde se escribiran los datos
 * len es la cantidad de bytes a leer
 */
void read_block_ext_eeprom(short bsb, long start, long len, int* data){
#warning "Sin probar!!"
short resp;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
int ControlByteR = CONTROL_BYTE_READ | ((int)bsb<<3);

   //esperamos que la memoria este lista
	do{
		i2c_start();
		resp = i2c_write(ControlByteW);
	}while(resp == EXT_EEPROM_NO_ACK);
	
	i2c_write(start>>8);		//address high
	i2c_write(start);			//address low

	i2c_start();				//start condition
	i2c_write(ControlByteR);	//control byte (read)
	
	for(long i = 0; i < len-1 ; i++){
		*data++ = i2c_read(EXT_EEPROM_RESPONSE_ACK);//read byte
	}
	*data = i2c_read(EXT_EEPROM_RESPONSE_NO_ACK);	//read last byte
	i2c_stop();					//stop condition
}

/*
 * Sirve para ver lo que hay en la memoria EEPROM y mostrarlo por puerto serie
 * bsb es uno de los dos bloques disponibles (block selection bit)
 * start es la direccion donde comenzamos a leer
 * end es la ultima direccion a leer
 */
#if definedinc(STDOUT)
void print_ext_eeprom(short bsb, long start, long len){
	printf("EEPROM_B%u (%Lu - %Lu):\r\n", bsb, start, start + len - 1);
	
	//imprimimos offset
	int resto = start%8;
	for(int y = 0; y < resto; y++){
		printf("   ");
	}
	
	//imprimimos valores
	for(long x = start; x < len; x++){
		//imprimo cambio de linea en los multiplos de 8
		if((x%8 == 0) && (x != start)){
			printf("\r\n");
		}
		
		printf("%02X ", read_ext_eeprom(bsb, x));	//imprimo valor
	}
	
	/*
	//lectura secuencial, mas rapida, pero necesitamos un buffer grande
	int valores[64];
	read_block_ext_eeprom(bsb, start, len, valores);
	
	for(long x = start; x < len; x++){
		//imprimo cambio de linea en los multiplos de 8
		if((x%8 == 0) && (x != start)){
			printf("\r\n");
		}
		
		printf("%02X ", valores[x]);	//imprimo valor
	}
	*/
	
	printf("\r\n\r\n");
}
#endif
