///////////////////////////////////////////////////////////////////////////
////   Library for a 24LC1025 serial EEPROM                             ////
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

#define EEPROM_SDA  PIN_B1
#define EEPROM_SCL  PIN_B0

#endif

#use i2c(master, sda=EEPROM_SDA, scl=EEPROM_SCL)

#define EEPROM_ADDRESS int16
#define EEPROM_SIZE   65535

void init_ext_eeprom()
{
   output_float(EEPROM_SCL);
   output_float(EEPROM_SDA);
}

void write_ext_eeprom(int1 bloque, int16 address, BYTE data)
{
   BYTE ControlByte=0b10100000;
   int1 status;

   if(bloque==1)
      ControlByte=0b10101000;

   i2c_start();
   i2c_write(ControlByte);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_write(data);
   i2c_stop();
   i2c_start();
   status=i2c_write(ControlByte);
   while(status==1)
   {
      i2c_start();
      status=i2c_write(ControlByte);
   }
}

BYTE read_ext_eeprom(int1 bloque, int16 address) {
   BYTE data;
   BYTE ControlByteW = 0b10100000;
   BYTE ControlByteR = 0b10100001;

   if(bloque == 1){
      ControlByteW = 0b10101000;
      ControlByteR = 0b10101001;
   }

   i2c_start();
   i2c_write(ControlByteW);
   i2c_write(address>>8);
   i2c_write(address);
   i2c_start();
   i2c_write(ControlByteR);  //bitset pone a '1' la posicion 0 de ControlByte
   data=i2c_read(0);
   i2c_stop();
   return(data);
}
