#ifndef DS1307
#define DS1307 

typedef struct {
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;
	uint8_t DayOfWeek;
	uint8_t Error;
} time_t;

extern unsigned char bin2bcd(unsigned char binary_value);
extern unsigned char bcd2bin(unsigned char bcd_value);
extern void ds1307_init(void);
extern unsigned char ds1307_write(time_t pData);
extern time_t ds1307_read(void); 
extern unsigned char BCD2UpperCh(unsigned char bcd);
extern unsigned char BCD2LowerCh(unsigned char bcd);

#endif