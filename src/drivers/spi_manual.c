#include <drivers/spi_manual.h>
#include <drivers/bcm2835min.h>
#include <drivers/lcd_vdg_api.h>
#include <common/stdlib.h>

//******************************************************************************
//      Functions
//******************************************************************************
void spi_man_init (void)
{
    bcm2835_gpio_fsel(TFT_CS_TCH, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_SCK, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_SDO, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_SDI, BCM2835_GPIO_FSEL_INPT);     

	bcm2835_gpio_write(TFT_SCK,0);
	bcm2835_gpio_write(TFT_SDO,1);
}

//******************************************************************************
void spi_man_send (unsigned char num)
{
	unsigned char count=0;
    bcm2835_gpio_write(TFT_SCK,0);

	for(count=0;count<8;count++)
	{
		if (num&0x80) 
		   bcm2835_gpio_write(TFT_SDO,1);
		else 
		   bcm2835_gpio_write(TFT_SDO,0);

		num = num<<1;

	    bcm2835_gpio_write(TFT_SCK,0);
	    Delayus(1);

	    bcm2835_gpio_write(TFT_SCK,1);
	    Delayus(1);
	}
}

//******************************************************************************

unsigned int spi_man_get (void)
{
	unsigned char count=0;
	unsigned int num=0;

	for(count=0;count<12;count++)
	{
		num = num<<1;

	    bcm2835_gpio_write(TFT_SCK,1);
	    Delayus(1);

	    bcm2835_gpio_write(TFT_SCK,0);
	    Delayus(1);
	
		if(bcm2835_gpio_lev(TFT_SDI) == 1) 
			num++;
	}

	return(num);
}

//******************************************************************************
void spi_man8_send (unsigned char num)
{
	unsigned char count=0;

    bcm2835_gpio_write(TFT_SCK,0);

	for(count=0;count<8;count++)
	{
		if (num&0x80) 
		   bcm2835_gpio_write(TFT_SDO,1);
		else 
		   bcm2835_gpio_write(TFT_SDO,1);

		num = num<<1;

	    bcm2835_gpio_write(TFT_SCK,0);
	    bcm2835_gpio_write(TFT_SCK,1);
	}

    bcm2835_gpio_write(TFT_SCK,0);
}

unsigned char spi_man8_get (void)
{
	unsigned char count=0, num=0;

    bcm2835_gpio_write(TFT_SCK,0);

	for(count=0;count<8;count++)
	{
		num = num<<1;

	    bcm2835_gpio_write(TFT_SCK,0);
	    bcm2835_gpio_write(TFT_SCK,1);
	
		if(bcm2835_gpio_lev(TFT_SDI) == 1) 
			num++;
	}

/*	TFT_SCK = 1;
	Delay1TCY ();
	Delay1TCY ();
	Delay1TCY ();

	TFT_SCK = 0;
	Delay1TCY ();
	Delay1TCY ();
	Delay1TCY ();*/
	
    bcm2835_gpio_write(TFT_SCK,0);

	return(num);
}
