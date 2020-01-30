/*
 * Libreria creada por Martin Andersen para IDEAA Lab
 * Basada en la libreria de CCS info
 */

/*
 * CONFIGIRACION
 * + Para que la libreria funcione hay que declarar los pines que se van a usar:
 *  Ejemplo:
 *	#define EEPROM_SCL	PIN_C0
 *	#define EEPROM_SDA	PIN_C1
 * 
 * + La memoria puede tener 4 direcciones I2C posibles. Se puede configurar
 *	usando EEPROM_I2C_ADDR_0 / EEPROM_I2C_ADDR_1 / EEPROM_I2C_ADDR_2 /EEPROM_I2C_ADDR_3
 *	Por defecto, si no se declara se usa la direccion 0.
 * 
 * + La escritura requiere unos 3-5 mS de tiempo. Aunque no hace falta quedarse
 *	esperando hasta que termine. Por defecto se espera, pero se puede declarar
 *	EEPROM_DONT_WAIT_ACK para que no espere y ganar tiempo.
 * 
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
 * POSIBLES MEJORAS
 * + Se podria esperar el ACK al inicio de la lectura y grabacion, y asi eliminarlo
 *	del final de la grabacion.
 * + Se puede implementar grabacion y lectura por bancos completos
 */

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
#define EEPROM_SIZE   65536		//en realidad el tamaño de la memoria es del doble
								//este es el tamaño de uno de los 2 bloques

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
void write_ext_eeprom(short bsb, long address, int data){
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	i2c_start();			//start condition
	i2c_write(ControlByteW);	//control byte (write)
	i2c_write(address>>8);	//address high
	i2c_write(address);		//address low
	i2c_write(data);		//data byte
	i2c_stop();				//stop condition
	
#ifndef EEPROM_DONT_WAIT_ACK
short resp;
	do{
		i2c_start();
		resp = i2c_write(ControlByteW);
	}while(resp == EXT_EEPROM_NO_ACK);
	
	i2c_stop();
#endif
}

void write1(short bsb, long address, int data){
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	i2c_start();			//start condition
	i2c_write(ControlByteW);	//control byte (write)
	i2c_write(address>>8);	//address high
	i2c_write(address);		//address low
	i2c_write(data);		//data byte
	i2c_stop();				//stop condition
	
short response;
	do{
		i2c_start();
		response = i2c_write(ControlByteW);
	}while(response == EXT_EEPROM_NO_ACK);
	
	i2c_stop();
}

void write2(short bsb, long address, int data){
short noACK;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	//wait for the memory to be ready
	do{
		i2c_start();
		noACK = i2c_write(ControlByteW);
	}while(noACK == TRUE);
	
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
void write_ext_eeprom(short bsb, long address, int* data, int len){
#warning "implementar"
}

/*
 * Lee de la EEPROM externa
 * Lee un byte del bloque y direccion que le pasamos
 */
int read_ext_eeprom(short bsb, long address){
   int data;
   int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
   int ControlByteR = CONTROL_BYTE_READ | ((int)bsb<<3);

   i2c_start();				//start condition
   i2c_write(ControlByteW);	//control byte (write)
   i2c_write(address>>8);	//address high
   i2c_write(address);		//address low
   
   i2c_start();				//start condition
   i2c_write(ControlByteR);	//control byte (read)
   data = i2c_read(0);		//read byte
   i2c_stop();				//stop condition
   
   return(data);
}

/*
 * Lee varios datos el mismo tiempo de la EEPROM
 * address es la posicion de inicio de lectura
 * data es un puntero a la variable donde se escribiran los datos
 * len es la cantidad de bytes a leer
 */
void read_ext_eeprom(short bsb, long address, int* data, int len){
#warning "implementar"
}
