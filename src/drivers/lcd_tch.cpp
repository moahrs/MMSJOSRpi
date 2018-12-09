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
#include <circle/interrupt.h>
#include <drivers/lcd_vdg.h>
#include <drivers/lcd_tch.h>
#include "common/mylib.h"

CLcdTch *CLcdTch::s_pThis = 0;

CLcdTch::CLcdTch (CInterruptSystem *mInterrupt, CLcdVdg *mLcdVdg)
: m_pInterruptSystem (mInterrupt),
  m_mLcdVdg (mLcdVdg)  
{
    s_pThis = this;
}

CLcdTch::~CLcdTch (void)
{
    s_pThis = 0;
    m_mLcdVdg = 0;
    m_pInterruptSystem = 0;
}

boolean CLcdTch::Initialize (void)
{
    spi_man_init();
    
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
    bcm2835_gpio_fsel(TFT_IRQ_TCH, BCM2835_GPIO_FSEL_INPT);     
    bcm2835_gpio_fen(TFT_IRQ_TCH);

    m_pInterruptSystem->ConnectIRQ (ARM_IRQ_GPIO0, lcd_tch_irq_handler, this);
    
    irqTchOn = 1;
    
    return TRUE;
}

void CLcdTch::lcd_tch_irq_handler(void) 
{
    unsigned char pRetAds;
    Coordinate AdsTch;

    pRetAds = Read_Ads7843(&AdsTch);        

    m_mLcdVdg->TFT_Dot(AdsTch.x,AdsTch.y,Red);
    m_mLcdVdg->TFT_Dot(AdsTch.x+1,AdsTch.y,Red);
    m_mLcdVdg->TFT_Dot(AdsTch.x,AdsTch.y+1,Red);
    m_mLcdVdg->TFT_Dot(AdsTch.x+1,AdsTch.y+1,Red);

    m_mLcdVdg->TCHVerif(pRetAds, AdsTch);

    bcm2835_gpio_set_eds(TFT_IRQ_TCH);
}

void CLcdTch::lcd_tch_irq_handler(void *pParam) 
{

    CLcdTch *pThis = (CLcdTch *) pParam;
    assert (pThis != 0);

    pThis->lcd_tch_irq_handler ();
}

int CLcdTch::Read_XY(unsigned char XY)
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


void CLcdTch::TP_GetAdXY(int *x,int *y)
{
    int adx,ady;

    adx = Read_XY(CHX);

    Delayns(300);

    ady = Read_XY(CHY);

	if (adx > 0) 
        *x = ((adx - 25) / 1.958f );    // Era 1.958f
	else
		*x = adx;

	if (ady > 0)
	    *y = ((ady - 55) / 1.375f );    // Era 1.375f  
	else
		*y = ady;
}

void CLcdTch::TP_DrawPoint(unsigned int Xpos,unsigned int Ypos, unsigned int color)
{
    (void) Xpos;
    (void) Ypos;
    (void) color;
//    Write_Command_Data(0x03, 0x1030);
    m_mLcdVdg->TFT_Dot(Xpos,Ypos,color);
    m_mLcdVdg->TFT_Dot(Xpos+1,Ypos,color);
    m_mLcdVdg->TFT_Dot(Xpos,Ypos+1,color);
    m_mLcdVdg->TFT_Dot(Xpos+1,Ypos+1,color);
}

unsigned char CLcdTch::Read_Ads7843(Coordinate * screenPtr)
{
    int m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
    int buffer[2][9]={{0},{0}};
/*    int ibb, iz, ix;
    int dfx, dfy;*/

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

        temp[0]=(buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
        temp[1]=(buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
        temp[2]=(buffer[0][6]+buffer[0][7]+buffer[0][8])/3;
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
                screenPtr->x=(temp[0]+temp[2])/2;
            else
                screenPtr->x=(temp[0]+temp[1])/2;
        }
        else if(m2<m1)
	            screenPtr->x=(temp[0]+temp[2])/2;
    	     else
        	    screenPtr->x=(temp[1]+temp[2])/2;

        temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
        temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
        temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;
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
                screenPtr->y=(temp[0]+temp[2])/2;
            else
                screenPtr->y=(temp[0]+temp[1])/2;
        }
        else if(m2<m1)
	            screenPtr->y=(temp[0]+temp[2])/2;
	         else
    	        screenPtr->y=(temp[1]+temp[2])/2;

        // Por causa do problema na divisão do Processador do Raspberry PI, tive que fazer essa GAMBI
/*        dfy = 0;
        iz = 0;
        ix = 0;
        for(ibb = 0; ibb <= screenPtr->y; ibb++) 
        {
            iz++;
            ix++;
            if (iz == 5) {
                dfy += 3;
                iz = 0;
            }

            if (ix == 6)
            {
                dfy++;
                ix = 0;
            }
        }
        if (iz > 0)
            dfy++;
        screenPtr->y = dfy;

        dfx = 0;
        iz = 0;
        ix = 0;
        for(ibb = 0; ibb <= screenPtr->x; ibb++) 
        {
            iz++;
            ix++;
            if (iz == 2) {
                dfx++;
                iz = 0;
            }

            if (ix == 31)
            {
                dfx++;
                ix = 0;
            }
        }
        screenPtr->x = dfx;*/

        return 1;
    }

    return 0;
}

unsigned int CLcdTch::getDisplayPoint(void)
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
