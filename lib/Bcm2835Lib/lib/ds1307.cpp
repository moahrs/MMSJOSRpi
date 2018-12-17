#include <stddef.h>
#include <stdint.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#include <drivers/ds1307.h>

unsigned char bin2bcd(unsigned char binary_value)
{
  unsigned char temp;
  unsigned char retval;

  temp = binary_value;
  retval = 0;

  while(1)
  {
    // Get the tens digit by doing multiple subtraction
    // of 10 from the binary value.
    if(temp >= 10)
    {
      temp -= 10;
      retval += 0x10;
    }
    else // Get the ones digit by adding the remainder.
    {
      retval += temp;
      break;
    }
  }

  return(retval);
}


// Input range - 00 to 99.
unsigned char bcd2bin(unsigned char bcd_value)
{
  unsigned char temp;

  temp = bcd_value;
  // Shifting upper digit right by 1 is same as multiplying by 8.
  temp >>= 1;
  // Isolate the bits for the upper digit.
  temp &= 0x78;

  // Now return: (Tens * 8) + (Tens * 2) + Ones

  return(temp + (temp >> 2) + (bcd_value & 0x0f));
} 

void ds1307_init(void) 
{    
  bcm2835_i2c_begin(); 
  bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);   // BitRate menor que 100 KHz
  bcm2835_i2c_setSlaveAddress(0b01101000); //address of DS1307. 
}

unsigned char ds1307_write(time_t pData) 
{ 
  char buf[9];
  unsigned char retError;

  buf[0] = 0x00;  
  buf[1] = bin2bcd(pData.Second);  
  buf[2] = bin2bcd(pData.Minute);  
  buf[3] = bin2bcd(pData.Hour);  
  buf[4] = bin2bcd(pData.DayOfWeek);  
  buf[5] = bin2bcd(pData.Day);  
  buf[6] = bin2bcd(pData.Month);  
  buf[7] = bin2bcd(pData.Year);  
  buf[8] = 0x00;  

  retError = bcm2835_i2c_write(buf,8);

  return retError;
} 

time_t ds1307_read(void) 
{
  char buf[7];
  time_t retData; 

  buf[0] = 0x00;

  retData.Error = bcm2835_i2c_write(buf,1); // Position the address pointer to 0. 
  if (!retData.Error)
  {
    retData.Error = bcm2835_i2c_read(buf,7); 
    if (!retData.Error)
    {
      retData.Second = buf[0]; 
      retData.Minute = buf[1]; 
      retData.Hour = buf[2]; 
      retData.DayOfWeek = buf[3]; 
      retData.Day = buf[4]; 
      retData.Month = buf[5]; 
      retData.Year = buf[6];
    }
  }

  return retData;
} 

unsigned char BCD2UpperCh(unsigned char bcd)
{
  return (((bcd & 0xF0) >> 4) | 0x30);
}

unsigned char BCD2LowerCh(unsigned char bcd)
{
	return ((bcd & 0x0F) | 0x30);
}

