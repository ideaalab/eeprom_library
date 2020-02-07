#ifndef EEPROM_256K
#define	EEPROM_256K
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
 * + La memoria puede tener 8 direcciones I2C posibles. Se puede configurar
 *	declarando un define EEPROM_I2C_ADDR_0 / EEPROM_I2C_ADDR_1 /
 *	EEPROM_I2C_ADDR_2 / EEPROM_I2C_ADDR_3 / EEPROM_I2C_ADDR_4 /
 *	EEPROM_I2C_ADDR_5 / EEPROM_I2C_ADDR_6 / EEPROM_I2C_ADDR_7
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
 * + write_ext_eeprom(long address, int data)
 *		escribe en la memoria:
 *		-address: posicion de memoria (0 - 32767)
 *		-data: valor a escribir (0 - 255)
 *
  * + void write_ext_eeprom_old(long address, int data)
 *		escribe en la memoria:
 *		-address: posicion de memoria (0 - 65535)
 *		-data: valor a escribir (0 - 255)
 *
 * + write_block_ext_eeprom(long start, long len, int* data)
 *		escribe un bloque memoria de un array:
 *		-start: posicion de comienzo de memoria (0 - 32767)
 *		-len: cantidad de datos a escribir (1 - 32767)
 *		-*data: puntero del array donde tenemos los datos guardados
 *			CUIDADO, la funcion no comprueba el tamaño del array. Tenemos
 *			que pasarle un array del al menos el tamaño "len" para que no
 *			lea variables adyacentes.
 *			Ejemplo:
 *			int valores[64];
 *			for(int x = 0; x < 64; x++){
 *				valores[x] = x;
 *			}
 *			write_block_ext_eeprom(0, 64, valores);
 * 
 * + read_ext_eeprom(long address)
 *		lee de la memoria y devuelve un INT con el valor leido:
 *		-address: posicion de memoria (0 - 32767)
 * 
 * + read_block_ext_eeprom(long start, long len, int* data)
 *		lee un bloque memoria a un array:
 *		-start: posicion de comienzo de memoria (0 - 32767)
 *		-len: cantidad de datos a leer (1 - 32767)
 *		-*data: puntero del array donde vamos a guardar los datos
 *			CUIDADO, la funcion no comprueba el tamaño del array. Tenemos
 *			que pasarle un array del al menos el tamaño "len" para que no
 *			sobreescriba variables adyacentes.
 *			Ejemplo:
 *			int valores[64];
 *			read_block_ext_eeprom(0, 0, 64, valores);
 * 
 * + print_ext_eeprom(long start, long len)
 *		muestra el contenido de la EEPROM por puerto serie
 *		requiere "#use rs232()" en programa, antes de llamar a la libreria
 *		-start: posicion de comienzo de memoria (0 - 32767)
 *		-len: cantidad de datos a leer (1 - 32767)
 ******************************************************************************/

/*
 * Control byte:
 * 0: R/W (1=R/0=W)
 * 1: Address 0
 * 2: Address 1
 * 3: Address 2
 * 4-7: Control code (0b1010)
 */

#ifndef EEPROM_SDA
#ERROR Hay que declarar el pin EEPROM_SDA
#endif

#ifndef EEPROM_SCL
#ERROR Hay que declarar el pin EEPROM_SCL
#endif

#if defined(EEPROM_I2C_ADDR_7)
#define CONTROL_BYTE_WRITE	0b10101110
#define CONTROL_BYTE_READ	0b10101111
#elif defined(EEPROM_I2C_ADDR_6)
#define CONTROL_BYTE_WRITE	0b10101100
#define CONTROL_BYTE_READ	0b10101101
#elif defined(EEPROM_I2C_ADDR_5)
#define CONTROL_BYTE_WRITE	0b10101010
#define CONTROL_BYTE_READ	0b10101011
#elif defined(EEPROM_I2C_ADDR_4)
#define CONTROL_BYTE_WRITE	0b10101000
#define CONTROL_BYTE_READ	0b10101001
#elif defined(EEPROM_I2C_ADDR_3)
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

#use i2c(MASTER, NOINIT, sda=EEPROM_SDA, scl=EEPROM_SCL, stream=EEPROM_I2C)

#define EEPROM_ADDRESS				long
#define EEPROM_PAGE_SIZE			64
#define EEPROM_SIZE					32768
#define EEPROM_PAGES				(EEPROM_SIZE/EEPROM_PAGE_SIZE)

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
void init_ext_eeprom(int speed){
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
		do{
			i2c_start(EEPROM_I2C);
		}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);

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
 * address es la direccion de memoria a escribir
 * data es el byte que escribiremos en la direccion
 */
void write_ext_eeprom(long address, int data){
	//esperamos a que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);
	
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
void write_ext_eeprom_old(long address, int data){
	i2c_start(EEPROM_I2C);				//start
	i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE);//control byte
	i2c_write(EEPROM_I2C, address>>8);	//address high
	i2c_write(EEPROM_I2C, address);		//address low
	i2c_write(EEPROM_I2C, data);		//data byte
	i2c_stop(EEPROM_I2C);				//stop
	
	//esperamos a que la memoria termine
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);
	i2c_stop(EEPROM_I2C);				//stop
}

/*
 * Escribe varios datos el mismo tiempo en la EEPROM
 * start es la posicion de inicio de grabacion
 * data es un puntero a los datos a escribir
 * len es la cantidad de bytes a escribir
 */
void write_block_ext_eeprom(long start, long len, int* data){
short primero;		//indica si es el primer valor del bloque
long end = start + len;

	do{
		//esperamos a que la memoria este lista
		do{
			i2c_start(EEPROM_I2C);
		}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);

		i2c_write(EEPROM_I2C, start>>8);	//address high
		i2c_write(EEPROM_I2C, start);		//address low

		primero = TRUE;
		
		while((primero == TRUE) || (start%EEPROM_PAGE_SIZE != 0)){
			i2c_Write(EEPROM_I2C, *data++);
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
int read_ext_eeprom(long address){
int data;

	//esperamos que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);
	
	i2c_write(EEPROM_I2C, address>>8);		//address high
	i2c_write(EEPROM_I2C, address);			//address low

	i2c_start(EEPROM_I2C);				//start condition
	i2c_write(EEPROM_I2C, CONTROL_BYTE_READ);	//control byte (read)
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
void read_block_ext_eeprom(long start, long len, int* data){
   //esperamos que la memoria este lista
	do{
		i2c_start(EEPROM_I2C);
	}while(i2c_write(EEPROM_I2C, CONTROL_BYTE_WRITE) == EXT_EEPROM_SLAVE_NO_ACK);
	
	i2c_write(EEPROM_I2C, start>>8);		//address high
	i2c_write(EEPROM_I2C, start);			//address low

	i2c_start(EEPROM_I2C);				//start condition
	i2c_write(EEPROM_I2C, CONTROL_BYTE_READ);	//control byte (read)
	
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
void print_ext_eeprom(long start, long len){
long end = start + len;
long page = start / EEPROM_PAGE_SIZE;
	
	printf("\r\nEEPROM (%Lu - %Lu):\r\n", start, end - 1);
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
		
		printf("%02X ", read_ext_eeprom(x));	//imprimo valor
	}
	
	printf("\r\n\r\n");
}
#endif
#endif