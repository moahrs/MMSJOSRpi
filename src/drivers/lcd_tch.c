/**************************************************************************************************
 *****                                                                                        *****
 *****  Name: Touch.cpp                                                                       *****
 *****  Ver.: 1.0                                                                             *****
 *****  Date: 04/01/2013                                                                      *****
 *****  Auth: Frank Vannieuwkerke                                                             *****
 *****        Erik Olieman                                                                    *****
 *****  Func: Touch driver for use with ADS7843                                               *****
 *****                                                                                        *****
 *****  Code based on Carlos E. Vidales tutorial :                                            *****
 *****  How To Calibrate Touch Screens                                                        *****
 *****  www.embedded.com/design/configurable-systems/4023968/How-To-Calibrate-Touch-Screens   *****
 *****                                                                                        *****
 **************************************************************************************************/
#include <stddef.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#include <drivers/spi_manual.h> 
#include <drivers/lcd_tch.h>
#include <drivers/lcd_vdg_api.h>

Coordinate  display;
Coordinate  screen;

#define THRESHOLD 2

Coordinate DisplaySample[3];
Coordinate ScreenSample[3];
Matrix matrix; 

char ptext[64];

unsigned char ASCCHR[16] = {
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,	// 0...9
0x41,0x42,0x43,0x44,0x45,0x46						// A...F
};

unsigned int tBlack = 0;
unsigned int tRed = 63488;
unsigned int tGreen = 2016;
unsigned int tBlue = 31;
unsigned int tWhite = 65535;
unsigned int tPurple = 61727;
unsigned int tYellow = 65504;
unsigned int tCyan = 2047;
unsigned int td_gray = 21130;
unsigned int tl_gray = 31727;
unsigned int count=0;

int __aeabi_idiv(int return_value)
{
    return 0;
}   

void Touch_Init(void)
{
    bcm2835_gpio_write(TFT_SCK,0);
    bcm2835_gpio_write(TFT_SDO,1);

	DisplaySample[0].x=45;
	DisplaySample[0].y=45;
	DisplaySample[1].x=45;
	DisplaySample[1].y=270;
	DisplaySample[2].x=190;
	DisplaySample[2].y=190;
	ScreenSample[0].x=45;
	ScreenSample[0].y=45;
	ScreenSample[1].x=45;
	ScreenSample[1].y=270;
	ScreenSample[2].x=190;
	ScreenSample[2].y=190;

    bcm2835_gpio_write(TFT_CS_TCH,1);
}

int Read_XY(unsigned char XY)
{
    unsigned int Temp;

    Temp = 0;

    bcm2835_gpio_write(TFT_SCK,0);
    bcm2835_gpio_write(TFT_SDO,1);

    bcm2835_gpio_write(TFT_CS_TCH,0);

    spi_man_send(XY);

    bcm2835_gpio_write(TFT_SCK,1);
    Delayns(300);

    bcm2835_gpio_write(TFT_SCK,0);
    Delayns(300);

    Temp = spi_man_get();

    bcm2835_gpio_write(TFT_CS_TCH,1);

    Temp >>= 3;
    Temp &= 0xfff;

    return(Temp);
}


void TP_GetAdXY(int *x,int *y)
{
    int adx,ady;
    double adxf, adyf;

    adx = Read_XY(CHX);

    Delayns(300);

    ady = Read_XY(CHY);

	if (adx > 0) 
    {
        adxf = (double)adx;
        *x = (int)((adxf - 25) / 2.603f );    // Era 1.958f
    }
	else
		*x = adx;

	if (ady > 0)
    {
        adyf = (double)ady;
	    *y = (int)((adyf - 55) / 1.375f );    // Era 1.375f  
    }
	else
		*y = ady;
}

void TP_DrawPoint(unsigned int Xpos,unsigned int Ypos, unsigned int color)
{
//    Write_Command_Data(0x03, 0x1030);
    TFT_Dot(Xpos,Ypos,color);
    TFT_Dot(Xpos+1,Ypos,color);
    TFT_Dot(Xpos,Ypos+1,color);
    TFT_Dot(Xpos+1,Ypos+1,color);
}

unsigned char Read_Ads7843(Coordinate * screenPtr)
{
    int m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
    int buffer[2][9]={{0},{0}};

    if (screenPtr == NULL) 
        screenPtr = &screen;

    count=0; 

    if (!bcm2835_gpio_lev(TFT_IRQ_TCH))
    {
        while(count < 9) 
        {
            TP_GetAdXY(TP_X,TP_Y);
            buffer[0][count]=TP_X[0];
            buffer[1][count]=TP_Y[0];
            count++;
        }
    }

    if(count==9)
    {
        count = 10;

        temp[0]=(int)((double)(buffer[0][0]+buffer[0][1]+buffer[0][2]) / 3);
        temp[1]=(int)((double)(buffer[0][3]+buffer[0][4]+buffer[0][5]) / 3);
        temp[2]=(int)((double)(buffer[0][6]+buffer[0][7]+buffer[0][8]) / 3);
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);

        if( (m0>THRESHOLD)  &&  (m1>THRESHOLD)  &&  (m2>THRESHOLD) ) return 0;

        if(m0<m1)
        {
            if(m2<m0)
                screenPtr->x=(int)((double)(temp[0]+temp[2]) / 2);
            else
                screenPtr->x=(int)((double)(temp[0]+temp[1]) / 2);
        }
        else if(m2<m1)
	            screenPtr->x=(int)((double)(temp[0]+temp[2]) / 2);
    	     else
        	    screenPtr->x=(int)((double)(temp[1]+temp[2]) / 2);

        temp[0]=(int)((double)(buffer[1][0]+buffer[1][1]+buffer[1][2]) / 3);
        temp[1]=(int)((double)(buffer[1][3]+buffer[1][4]+buffer[1][5]) / 3);
        temp[2]=(int)((double)(buffer[1][6]+buffer[1][7]+buffer[1][8]) / 3);
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);

        if( (m0>THRESHOLD)  &&  (m1>THRESHOLD)  &&  (m2>THRESHOLD) ) return 0;

        if(m0<m1)
        {
            if(m2<m0)
                screenPtr->y=(int)((double)(temp[0]+temp[2]) / 2);
            else
                screenPtr->y=(int)((double)(temp[0]+temp[1]) / 2);
        }
        else if(m2<m1)
	            screenPtr->y=(int)((double)(temp[0]+temp[2]) / 2);
	         else
    	        screenPtr->y=(int)((double)(temp[1]+temp[2]) / 2);

        return 1;
    }

    return 0;
}

unsigned int getDisplayPoint(void)
{
    unsigned int retTHRESHOLD = 0 ;

    if( matrix.Divider != 0 )
    {
        // XD = AX+BY+C
        display.x = ( (matrix.An * screen.x) +
                          (matrix.Bn * screen.y) +
                           matrix.Cn
                         ) / matrix.Divider ;
        // YD = DX+EY+F
        display.y = ( (matrix.Dn * screen.x) +
                          (matrix.En * screen.y) +
                           matrix.Fn
                         ) / matrix.Divider ;
    }
    else
    {
        retTHRESHOLD = 1;
    }

    return(retTHRESHOLD);
}
