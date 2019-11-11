///////////////////////////////////////////////////////////////////////////
////   Library for a 24LC256 serial EEPROM                             ////
////                                                                   ////
////   init_ext_eeprom();    Call before the other functions are used  ////
////                                                                   ////
////   write_ext_eeprom(a, d);  Write the byte d to the address a      ////
////                                                                   ////
////   d = read_ext_eeprom(a);   Read the byte d from the address a    ////
////                                                                   ////
////   The main program may define eeprom_sda                          ////
////   and eeprom_scl to override the defaults below.                  ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////


#ifndef EEPROM_SDA
//#define EEPROM_SDA	PIN_A0
#ERROR Hay que declarar el pin EEPROM_SDA
#endif

#ifndef EEPROM_SCL
//#define EEPROM_SCL	PIN_A1
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
#elif defined(EEPROM_I2C_ADDR_0)
#define CONTROL_BYTE_WRITE	0b10100000
#define CONTROL_BYTE_READ	0b10100001
#else
#define CONTROL_BYTE_WRITE	0b10100000
#define CONTROL_BYTE_READ	0b10100001
#endif

/*
#warning "Control byte escritura I2C:" CONTROL_BYTE_WRITE
#warning "Control byte lectura I2C:" CONTROL_BYTE_READ
*/

//hay que forzar I2C por software, sino no funciona ¿?
#use i2c(FORCE_SW, master, sda=EEPROM_SDA, scl=EEPROM_SCL)

#define EEPROM_ADDRESS long int
#define EEPROM_SIZE   32768

/*
 * Incializa la EEPROM externa
 */
void init_ext_eeprom(void){
   output_float(EEPROM_SCL);
   output_float(EEPROM_SDA);
}

/*
 * Escribe en la EEPROM externa
 * address es la direccion de memoria a escribir
 * data es el byte que escribiremos en la direccion
 */
void write_ext_eeprom(long address, int data){
   short int status;
   i2c_start();
   i2c_write(CONTROL_BYTE_WRITE);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
   i2c_start();
   status=i2c_write(CONTROL_BYTE_WRITE);
   while(status==1) {
      i2c_start();
	  status=i2c_write(CONTROL_BYTE_WRITE);
   }
   i2c_stop();
}

/*
 * Escribe varios datos el mismo tiempo en la EEPROM
 * address es la posicion de inicio de grabacion
 * data es un puntero a los datos a escribir
 * len es la cantidad de bytes a escribir
 */
void write_ext_eeprom(long address, int* data, int len){
#warning "implementar"
}

/*
 * Lee de la EEPROM externa
 * Lee un byte de la direccion que le pasamos
 */
int read_ext_eeprom(long address){
   BYTE data;
   i2c_start();
   i2c_write(CONTROL_BYTE_WRITE);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_start();
   i2c_write(CONTROL_BYTE_READ);
   data=i2c_read(0);
   i2c_stop();
   return(data);
}

/*
 * Lee varios datos el mismo tiempo de la EEPROM
 * address es la posicion de inicio de lectura
 * data es un puntero a la variable donde se escribiran los datos
 * len es la cantidad de bytes a leer
 */
void read_ext_eeprom(long address, int* data, int len){
#warning "implementar"
}
