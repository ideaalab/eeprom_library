#ifndef EEPROM_1024K
#define	EEPROM_1024K
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
 * + init_ext_eeprom(int speed)
 *		llamar antes de usar la libreria para inicializar el I2C
 *		-speed: usar una de las constantes para escoger velocidad
 *		EXT_EEPROM_100KHZ / EXT_EEPROM_400KHZ
 * 
 * + erase_ext_eeprom(void)
 *		borra todo el contenido de la EEPROM (0xFF)
 * 
 * + write_ext_eeprom(short bsb, long address, int data)
 *		escribe en la memoria:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-address: posicion de memoria (0 - 65535)
 *		-data: valor a escribir (0 - 255)
 * 
 * + void write_ext_eeprom_old(short bsb, long address, int data)
 *		escribe en la memoria:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-address: posicion de memoria (0 - 65535)
 *		-data: valor a escribir (0 - 255)
 *
 * + write_block_ext_eeprom(short bsb, long start, long len, int* data)
 *		escribe un bloque memoria de un array:
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-start: posicion de comienzo de memoria (0 - 65535)
 *		-len: cantidad de datos a escribir (1 - 65535)
 *		-*data: puntero del array donde tenemos los datos guardados
 *			CUIDADO, la funcion no comprueba el tamaño del array. Tenemos
 *			que pasarle un array del al menos el tamaño "len" para que no
 *			lea variables adyacentes.
 *			Ejemplo:
 *			int valores[64];
 *			for(int x = 0; x < 64; x++){
 *				valores[x] = x;
 *			}
 *			write_block_ext_eeprom(EXT_EEPROM_BANK_0, 0, 64, valores);
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
 * 
 * + print_ext_eeprom(short bsb, long start, long len)
 *		muestra el contenido de la EEPROM por puerto serie
 *		requiere "#use rs232()" en programa, antes de llamar a la libreria
 *		-bsb: banco de memoria (mitad inferior [0] o mitad superior [1])
 *		se pueden usar las constantes: EXT_EEPROM_BANK_0 y EXT_EEPROM_BANK_1
 *		-start: posicion de comienzo de memoria (0 - 65535)
 *		-len: cantidad de datos a leer (1 - 65535)
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

#define CONTROL_BYTE_WRITE_B0	CONTROL_BYTE_WRITE
#define CONTROL_BYTE_WRITE_B1	(CONTROL_BYTE_WRITE | (int)1<<3)

#use i2c(MASTER, NOINIT, sda=EEPROM_SDA, scl=EEPROM_SCL, stream=EEPROM_I2C)

#define EEPROM_ADDRESS				long
#define EEPROM_PAGE_SIZE			128
#define EEPROM_SIZE					65536	//en realidad el tamaño de la memoria es del doble, este es el tamaño de uno de los 2 bloques
#define EEPROM_PAGES				(EEPROM_SIZE / EEPROM_PAGE_SIZE)

#define EXT_EEPROM_BANK_0			0		//banco 0, 65536 bytes disponibles
#define EXT_EEPROM_BANK_1			1		//banco 1, otros 65536 bytes disponibles

//respuestas devueltas por i2c_write()
#define EXT_EEPROM_SLAVE_ACK		0		//slave responde que si esta listo
#define EXT_EEPROM_SLAVE_NO_ACK		1		//slave responde que no esta listo

//usadas en i2c_read("AQUI")
#define EXT_EEPROM_MASTER_ACK		1		//si queremos seguir leyendo datos
#define EXT_EEPROM_MASTER_NO_ACK	0		//si queremos terminar la lectura

//velocidad del reloj:
#define EXT_EEPROM_100KHZ			0
#define EXT_EEPROM_400KHZ			1

#define EMPTY_EEPROM_VAL			0xFF	//valor de memoria vacia

/*
 * Incializa la EEPROM externa
 */
void init_ext_eeprom(short speed){
	output_float(EEPROM_SCL);
	output_float(EEPROM_SDA);

	switch(speed){
		case EXT_EEPROM_100KHZ:
			i2c_init(EEPROM_I2C, 100000);
			break;
			
		case EXT_EEPROM_400KHZ:
			i2c_init(EEPROM_I2C, 400000);
			break;
	}
}

/*
 * Borra todo el contenido de la EEPROM
 */
void erase_ext_eeprom(void){
long pos = 0;
		
	for(long x = 0; x < EEPROM_PAGES; x++){
		/* BLOQUE 0 */
		do{
			i2c_start(EEPROM_I2C);
		}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE_B0) == EXT_EEPROM_SLAVE_NO_ACK);

		i2c_write(EEPROM_I2C, pos>>8);	//address high
		i2c_write(EEPROM_I2C, pos);		//address low

		for(int y = 0; y < EEPROM_PAGE_SIZE; y++){
			i2c_write(EEPROM_I2C, EMPTY_EEPROM_VAL);
		}

		i2c_stop(EEPROM_I2C);

		/* BLOQUE 1 */
		do{
			i2c_start(EEPROM_I2C);
		}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE_B1) == EXT_EEPROM_SLAVE_NO_ACK);

		i2c_write(EEPROM_I2C, pos>>8);	//address high
		i2c_write(EEPROM_I2C, pos);		//address low

		for(int y = 0; y < EEPROM_PAGE_SIZE; y++){
			i2c_write(EEPROM_I2C, EMPTY_EEPROM_VAL);
		}

		i2c_stop(EEPROM_I2C);

		pos = pos + EEPROM_PAGE_SIZE;
	}
}

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
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);

	//esperamos a que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, ControlByteW) == EXT_EEPROM_SLAVE_NO_ACK);
	
	i2c_write(EEPROM_I2C, address>>8);	//address high
	i2c_write(EEPROM_I2C, address);		//address low
	i2c_write(EEPROM_I2C, data);		//data byte
	i2c_stop(EEPROM_I2C);				//stop condition
}

/*
 * Escribe en la EEPROM externa y ESPERO A QUE TERMINE DE ESCRIBIR
 * bsb es uno de los dos bloques disponibles (block selection bit)
 * address es la direccion de memoria a escribir
 * data es el byte que escribiremos en la direccion
 */
void write_ext_eeprom_old(short bsb, long address, int data){
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
	
	i2c_start(EEPROM_I2C);				//start
	i2c_write(EEPROM_I2C, ControlByteW);//control byte
	i2c_write(EEPROM_I2C, address>>8);	//address high
	i2c_write(EEPROM_I2C, address);		//address low
	i2c_write(EEPROM_I2C, data);		//data byte
	i2c_stop(EEPROM_I2C);				//stop
	
	//esperamos a que la memoria termine
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, ControlByteW) == EXT_EEPROM_SLAVE_NO_ACK);
	i2c_stop(EEPROM_I2C);				//stop
}

/*
 * Escribe varios datos el mismo tiempo en la EEPROM
 * bsb es uno de los dos bloques disponibles
 * start es la posicion de inicio de grabacion
 * data es un puntero a los datos a escribir
 * len es la cantidad de bytes a escribir
 */
void write_block_ext_eeprom(short bsb, long start, long len, int* data){
short primero;		//indica si es el primer valor del bloque
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
long end = start + len;

	do{
		//esperamos a que la memoria este lista
		do{
			i2c_start(EEPROM_I2C);
		}while(i2c_write(EEPROM_I2C, ControlByteW) == EXT_EEPROM_SLAVE_NO_ACK);

		i2c_write(EEPROM_I2C, start>>8);	//address high
		i2c_write(EEPROM_I2C, start);		//address low
		
		primero = TRUE;
		
		while((primero == TRUE) || (start%EEPROM_PAGE_SIZE != 0)){
			i2c_write(EEPROM_I2C, *data++);
			start++;
			primero = FALSE;
		};
		
		i2c_stop(EEPROM_I2C);
	}while(start < end);
}

/*
 * Lee de la EEPROM externa
 * Lee un byte del bloque y direccion que le pasamos
 * Comprueba que haya terminado cualquier grabacion antes de leer.
 */
int read_ext_eeprom(short bsb, long address){
int data;
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
int ControlByteR = CONTROL_BYTE_READ | ((int)bsb<<3);

	//esperamos que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, ControlByteW) == EXT_EEPROM_SLAVE_NO_ACK);
	
	i2c_write(EEPROM_I2C, address>>8);		//address high
	i2c_write(EEPROM_I2C, address);			//address low

	i2c_start(EEPROM_I2C);					//start condition
	i2c_write(EEPROM_I2C, ControlByteR);	//control byte (read)
	data = i2c_read(EEPROM_I2C, EXT_EEPROM_MASTER_NO_ACK);	//read byte
	i2c_stop(EEPROM_I2C);					//stop condition

	return(data);
}

/*
 * Lee varios datos el mismo tiempo de la EEPROM
 * address es la posicion de inicio de lectura
 * data es un puntero a la variable donde se escribiran los datos
 * len es la cantidad de bytes a leer
 */
void read_block_ext_eeprom(short bsb, long start, long len, int* data){
int ControlByteW = CONTROL_BYTE_WRITE | ((int)bsb<<3);
int ControlByteR = CONTROL_BYTE_READ | ((int)bsb<<3);

   //esperamos que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, ControlByteW) == EXT_EEPROM_SLAVE_NO_ACK);
	
	i2c_write(EEPROM_I2C, start>>8);		//address high
	i2c_write(EEPROM_I2C, start);			//address low

	i2c_start(EEPROM_I2C);					//start condition
	i2c_write(EEPROM_I2C, ControlByteR);	//control byte (read)
	
	for(long i = 0; i < len-1 ; i++){
		*data++ = i2c_read(EEPROM_I2C, EXT_EEPROM_MASTER_ACK);	//read byte
	}
	
	*data = i2c_read(EEPROM_I2C, EXT_EEPROM_MASTER_NO_ACK);		//read last byte
	i2c_stop(EEPROM_I2C);					//stop condition
}

/*
 * Sirve para ver lo que hay en la memoria EEPROM externa y mostrarlo
 * por puerto serie
 * bsb es uno de los dos bloques disponibles (block selection bit)
 * start es la direccion donde comenzamos a leer
 * len es la cantidad de bytes a leer
 */
#if definedinc(STDOUT)
void print_ext_eeprom(short bsb, long start, long len){
long end = start + len;
long page = start / EEPROM_PAGE_SIZE;
	
	printf("\r\nEEPROM BLOCK %u (%Lu - %Lu):\r\n", bsb, start, start + len - 1);
	printf("\r\n> Page %Lu\r\n", page);
	
	//imprimimos offset
	int resto = start%8;
	for(int y = 0; y < resto; y++){
		printf("   ");
	}
	
	//imprimimos valores
	for(long x = start; x < end; x++){
		//imprimo cambio de linea en los multiplos de 8
		if((x%8 == 0) && (x != start)){
			printf("\r\n");
			
			//imprimo otro cambio de linea en los cambios de pagina
			if(x%EEPROM_PAGE_SIZE == 0){
				page = x / EEPROM_PAGE_SIZE;
				printf("\r\n> Page %Lu\r\n", page);
			}
		}
		
		printf("%02X ", read_ext_eeprom(bsb, x));	//imprimo valor
	}
	
	printf("\r\n\r\n");
}
#endif
#endif