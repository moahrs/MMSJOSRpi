#define TFT_CS_TCH RPI_V2_GPIO_P1_24
#define TFT_SCK RPI_V2_GPIO_P1_23
#define TFT_SDO RPI_V2_GPIO_P1_19
#define TFT_SDI RPI_V2_GPIO_P1_21

extern void spi_man_init (void);
extern void spi_man_send (unsigned char c);     // Send byte
extern unsigned int spi_man_get (void);        // Wait 32 us, then receive Byte
extern void spi_man8_send (unsigned char c);     // Send byte
extern unsigned char spi_man8_get (void);
