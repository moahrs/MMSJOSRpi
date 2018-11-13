#include <common/stdlib.h>
#include <common/stdio.h>
#include <drivers/bcm2835min.h>
#include <drivers/lcd_tch.h>
#include <drivers/lcd_vdg.h>
#include <drivers/lcd_vdg_api.h>
#include <drivers/lcd_vdg_fontes.h>  
#include <kernel/interrupts.h>
#include "common/mylib.h"

static void lcd_tch_irq_handler(void) {
    if (irqTchOn)
    {
        irqTchOn = 0;
        TCHVerif();
    }
}

static void lcd_tch_irq_clearer(void) {
    if (bcm2835_gpio_eds(TFT_IRQ_TCH))
    {
      irqTchOn = 1;
      bcm2835_gpio_set_eds(TFT_IRQ_TCH);
    }
}

int commVDG(unsigned char *vparam)
{
  unsigned char pconv1, pconv2;

  pconv1 = *(vparam + 2);
  pconv2 = *(vparam + 3);
  vyy = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  pconv1 = *(vparam + 4);
  pconv2 = *(vparam + 5);
  vxx = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  pconv1 = *(vparam + 6);
  pconv2 = *(vparam + 7);
  vyyf = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  pconv1 = *(vparam + 8);
  pconv2 = *(vparam + 9);
  vxxf = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  dfont = *(vparam + 6);

  pconv1 = *(vparam + 7);
  pconv2 = *(vparam + 8);
  fcor = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  pconv1 = *(vparam + 9); 
  pconv2 = *(vparam + 10);
  bcor = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

  pconv1 = *(vparam + 10);
  pconv2 = *(vparam + 11);
  fcorgraf = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);

	switch (*(vparam + 1)) {
		case 0xD0: // Clear Screen
        pconv1 = *(vparam + 2);
        pconv2 = *(vparam + 3);
        fcorgraf = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);
        TFT_Fill(fcorgraf);
        break;
		case 0xD1: // Write String
        ix = 11;
        while (*(vparam + ix) != 0) {
           cchar = *(vparam + ix);
           *(vparam + (ix - 11)) = cchar;
           ix++;
        }

        *(vparam + (ix - 11)) = '\0';
		   
        TFT_Text((char*)vparam,vxx,vyy,dfont,fcor,bcor);
 		    break;
		case 0xD2: // Write Char
		    if (*(vparam + 11) == 0x08) {
          TFT_Char(' ',vxx,vyy,dfont,fcor,bcor);
          vxcur = vxx;
          vycur = vyy;
        }      
        else {
          TFT_Char(*(vparam + 11),vxx,vyy,dfont,fcor,bcor);
          vxcur = vxx;
          vycur = vyy;
          vycur += 8;
        }    
		    break;
		case 0xD3: //Draw a Box
        TFT_Box(vxx,vyy,vxxf,vyyf,fcorgraf);
		    break;
		case 0xD4: //Draw a Line
        TFT_Line(vxx,vyy,vxxf,vyyf,fcorgraf);
		    break;
    case 0xD5: // Draw a Rectangle
        TFT_Rectangle(vxx,vyy,vxxf,vyyf,fcorgraf);
        break;
    case 0xD6: // Draw a Circle
        TFT_Circle(vxx,vyy,*(vparam + 6),*(vparam + 7),bcor);
        break;
    case 0xD7: // Set a point in LCD 
        TFT_Dot(vxx,vyy,fcor);
        break;
    case 0xD8: // 0 - Hidden Cursor, 1 - Show Cursor
        vcur = *(vparam + 2);
        fcorcur = fcor;
        bcorcur = bcor;
        
        if (!vcur)
          TFT_Char(' ',vxcur,vycur,8,fcorcur,bcorcur);
        else
          TFT_Char('_',vxcur,vycur,8,fcorcur,bcorcur);
 		    
        break;                     
    case 0xD9: // Vertical Scroll
		    pconv1 = *(vparam + 3);
        pconv2 = *(vparam + 4);
		    bcor = ((pconv1 << 8) & 0xFF00) | (pconv2 & 0x00FF);
        TFT_Scroll(*(vparam + 2));
        break;
    case 0xDB: // Ativar ou Desativa o Teclado e caso seja ativar, já deixa para re-ativar on Touch.
        if (*(vparam + 6) == 0x01) {
        	vkeyteste = 1;
        	pshowkeyontouch = 1;
        	vtipofimkey = *(vparam + 7);
        	vcapson = *(vparam + 8);		
        	vtipokey = *(vparam + 9);
        	keyvxx = vxx;
        	keyvyy = vyy;

        	ShowKeyboard(keyvyy, keyvxx, 1);
        }
        else if (*(vparam + 6) == 0x02) {
        	vkeyteste = 0;
        	pshowkeyontouch = 0;
        	HideKeyboard(keyvyy, keyvxx);
        }
        break;
    case 0xDC: // Retorna teclado
        *vparam = keyBuffer[0];
        if (icount > 0) {
        	for(ix = 1; ix <= icount; ix++) {
        		iz = keyBuffer[ix];
        		keyBuffer[ix - 1] = iz;
        	}
        	keyBuffer[icount] = 0x00;
        	icount--;
        }
        else
        	KBD_DATARDY = 0x01;

        break;
    case 0xDD: // Retorna posicao se tocado fora do teclado
        *vparam = (pyyyreturn[0] & 0xFF00) >> 8;
        *(vparam + 1) = (pyyyreturn[0] & 0x00FF);
        *(vparam + 2) = (pxxxreturn[0] & 0xFF00) >> 8;
        *(vparam + 3) = (pxxxreturn[0] & 0x00FF);

        if (icountp > 0) {
        	for(ix = 1; ix <= icountp; ix++) {
        		iy = pxxxreturn[ix];
        		pxxxreturn[ix - 1] = iy;
        		iy = pyyyreturn[ix];
        		pyyyreturn[ix - 1] = iy;
        	}
        	pxxxreturn[icountp] = 0x00;
        	pyyyreturn[icountp] = 0x00;
        	icountp--;
        }

        break;
		case 0xDE:
        iz = *(vparam + 2);
        iz = iz * 64;
        for(ix = 3; ix <= 130; ix += 2) {
        	iy = *(vparam + ix);
        	pvideo[iz] = ((iy & 0x00FF) << 8);
        	iy = *(vparam + (ix + 1));
        	pvideo[iz] |= (iy & 0x00FF);
        	iz++;
        }
        break;
		case 0xDF:
        // vxxf e vyyf vao funcionar como dim_x e dim_y respectivamente
        TFT_Image(vxx, vyy, vxxf, vyyf, (unsigned int*)pvideo);
        break;
		case 0xEA:
        // vxxf e vyyf vao funcionar como width e height respectivamente, e fcorgraf como page
        TFT_SaveScreen(fcorgraf, vxx, vyy, vxxf, vyyf);
        break;
		case 0xEB:
        // vxxf e vyyf vao funcionar como width e height respectivamente
        TFT_RestoreScreen(fcorgraf, vxx, vyy, vxxf, vyyf);
        break;
		case 0xEC:
        // vxxf e vyyf vao funcionar como dim_x e dim_y respectivamente
        TFT_InvertRect(vxx, vyy, vxxf, vyyf);
        break;
		case 0xED:
        // Verifica Touch
        *vparam = TCHVerif();
        break;
    case 0xEF: // Return Status Information
        *vparam = 0b10000010;    // D7 (0-16F877A, 1-18F4550)
                                 // D1 (0-No Touch, 1-Touch)
                                 // D0 (0-LCDG)
        *(vparam + 1) = (y_max & 0xFF00) >> 8;
        *(vparam + 2) = (y_max & 0x00FF);
        *(vparam + 3) = (x_max & 0xFF00) >> 8;
        *(vparam + 4) = (x_max & 0x00FF);

        break;
    case 0xF0:
        // Mostra / Esconde Cursor
        if (vcur) {
            if (vcurcont == 0x05000) {
                if (vxcur < x_max && vycur < y_max) {
                    TFT_Char('_',vxcur,vycur,8,fcorcur,bcorcur);
                }
            }
            else if (vcurcont == 0x10000 || preadywr) {
                if (vxcur < x_max && vycur < y_max) {
                    TFT_Char(' ',vxcur,vycur,8,fcorcur,bcorcur);
                }
                vcurcont = 0;
            }
            
            vcurcont++;
        }    
                
        break;
	}

	return 1;		
}

//-------------------------------------------------------------------
int TCHVerif(void) {
	int iRetAux = 0;

	if (Read_Ads7843(&screen_cal) && screen_cal.x != 0 && screen_cal.y != 0) {
		ptec[1] = 0x00;
		ptec[2] = 0x00;

    ReturnKeyboard(keyvyy, keyvxx, screen_cal.y, screen_cal.x);

		if (vtimetec == 0) {
			if (pshowkeyontouch && !pkeyativo) {
				if (!vkeyrep) {
					vkeyrep = 1;
					ptec[1] = 0xFF;
					ptec[2] = 0x00;
					ShowKeyboard(keyvyy, keyvxx, 1);
				}
			}	
			else if (pkeyativo) {
				if (!vkeyrep) {
					vkeyrep = 1;
	
					ReturnKeyboard(keyvyy, keyvxx, screen_cal.y, screen_cal.x);
		
					if (ptec[1]) {
						if (ptec[2] <= 3) {
							if (ptec[2] == 1) {
								if (vtipokey == 0)
									vtipokey = 1;
								else if (vtipokey == 1)
									vtipokey = 2;
								else
									vtipokey = 0;
							}
							else if (ptec[2] == 2)
								vcapson = ~vcapson;
		
							if (ptec[2] != 3) 
								ShowKeyboard(keyvyy, keyvxx, 1);
							else {
								HideKeyboard(keyvyy, keyvxx);
							}
						}
						else {
							keyBuffer[icount] = ptec[2];
						}
					}
				}
			}
	
			if ((!ptec[1]) || (ptec[1] && ptec[2] > 3)) {
				if (!ptec[1]) {
					keyBuffer[icount] = 0xFF;
					pxxxreturn[icountp] = screen_cal.y;
					pyyyreturn[icountp] = screen_cal.x;
					if (icountp < 16)
						icountp++;
				}
	
				if (icount < 16)
					icount++;

				vtimetec = 0;

				iRetAux = 1;
			}
		}
	}
	else {
		if (vkeyrep)
			vkeyrep = 0;
	}

	return iRetAux;
}

//-------------------------------------------------------------------
// Basic Graphic Functions
//-------------------------------------------------------------------
void Write_Bytes_GPIO(unsigned int wByte)
{
	unsigned int tByte;

  	bcm2835_gpio_write(LS373_LE,0);

  	// D8 a D15
	tByte = wByte >> 8;
  	bcm2835_gpio_write(TFT_D0,(tByte & 0b00000001));
  	bcm2835_gpio_write(TFT_D1,((tByte & 0b00000010) >> 1));
  	bcm2835_gpio_write(TFT_D2,((tByte & 0b00000100) >> 2));
  	bcm2835_gpio_write(TFT_D3,((tByte & 0b00001000) >> 3));
  	bcm2835_gpio_write(TFT_D4,((tByte & 0b00010000) >> 4));
  	bcm2835_gpio_write(TFT_D5,((tByte & 0b00100000) >> 5));
  	bcm2835_gpio_write(TFT_D6,((tByte & 0b01000000) >> 6));
  	bcm2835_gpio_write(TFT_D7,((tByte & 0b10000000) >> 7));
  	bcm2835_gpio_write(LS373_LE,1);
  	Delayns(15);
  	bcm2835_gpio_write(LS373_LE,0);

  	// D0 a D7
	tByte = wByte & 0x00FF;
  	bcm2835_gpio_write(TFT_D0,(tByte & 0b00000001));
  	bcm2835_gpio_write(TFT_D1,((tByte & 0b00000010) >> 1));
  	bcm2835_gpio_write(TFT_D2,((tByte & 0b00000100) >> 2));
  	bcm2835_gpio_write(TFT_D3,((tByte & 0b00001000) >> 3));
  	bcm2835_gpio_write(TFT_D4,((tByte & 0b00010000) >> 4));
  	bcm2835_gpio_write(TFT_D5,((tByte & 0b00100000) >> 5));
  	bcm2835_gpio_write(TFT_D6,((tByte & 0b01000000) >> 6));
  	bcm2835_gpio_write(TFT_D7,((tByte & 0b10000000) >> 7));
}

void Write_Command(unsigned int wcommand)
{
  	bcm2835_gpio_write(TFT_RD,1);
  	bcm2835_gpio_write(TFT_DC,0);
  	Write_Bytes_GPIO(wcommand);
  	bcm2835_gpio_write(TFT_WR,0);
  	Delayns(35);
  	bcm2835_gpio_write(TFT_WR,1);
}

//-------------------------------------------------------------------
void Write_Data(unsigned int wdata)
{
    if (pBaseVideoAddrX <= 319 && pBaseVideoAddrY <= 239) 
    {
        pBaseVideoMem[0][pBaseVideoAddrX][pBaseVideoAddrY] = wdata;
    }

  	bcm2835_gpio_write(TFT_RD,1);
  	bcm2835_gpio_write(TFT_DC,1); 
  	Write_Bytes_GPIO(wdata);
  	bcm2835_gpio_write(TFT_WR,0);
  	Delayns(35); 
  	bcm2835_gpio_write(TFT_WR,1);
}

//-------------------------------------------------------------------
unsigned int Read_Data(void) {
	// Ler memoria de video em RAM interna
    return 0;
}

//-------------------------------------------------------------------
void Write_Command_Data(unsigned int wcommand,unsigned int Wdata)
{
   Write_Command(wcommand);
   pBaseVideoAddrX = 320;
   pBaseVideoAddrY = 240;
   Write_Data(Wdata);
}

//-------------------------------------------------------------------
void TFT_Set_Address(unsigned int PX1,unsigned int PY1,unsigned int PX2,unsigned int PY2)
{
    pBaseVideoAddrX = PY1;
    pBaseVideoAddrY = PX1;

    Write_Command_Data(0x44,(PX2 << 8) + PX1 );   //Column address start2
    Write_Command_Data(0x45,PY1);                 //Column address start1
    Write_Command_Data(0x46,PY2);                 //Column address end2
    Write_Command_Data(0x4E,PX1);                 //Column address end1
    Write_Command_Data(0x4F,PY1);                 //Row address start2
    Write_Command(0x22);
}

void TFT_EN_INT(void)
{
    register_irq_handler(GPIO_INT0, lcd_tch_irq_handler, lcd_tch_irq_clearer);
}

//-------------------------------------------------------------------
void TFT_Init(void)
{
  	bcm2835_gpio_fsel(TFT_DC, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_RST, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_CS, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_RD, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_WR, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D0, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D1, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D2, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D3, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D4, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D5, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D6, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_D7, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(LS373_LE, BCM2835_GPIO_FSEL_OUTP); 
    bcm2835_gpio_fsel(TFT_IRQ_TCH, BCM2835_GPIO_FSEL_INPT);     
    bcm2835_gpio_fen(TFT_IRQ_TCH);
    
  	bcm2835_gpio_write(TFT_DC,1);
  	bcm2835_gpio_write(TFT_CS,1);
  	bcm2835_gpio_write(TFT_RD,1);
  	bcm2835_gpio_write(TFT_WR,1);
  	bcm2835_gpio_write(LS373_LE,0);

  	bcm2835_gpio_write(TFT_RST,1);
    Delayms(5);   // 5mS
  	bcm2835_gpio_write(TFT_RST,0);
    Delayms(15);  // 15mS
  	bcm2835_gpio_write(TFT_RST,1);
    Delayms(15);  // 15mS
    bcm2835_gpio_write(TFT_CS,0);

    Write_Command_Data(0x0000,0x0001); // Oscillation Start
    
    // Power controls
    Write_Command_Data(0x0003,0xA8A4);
    Write_Command_Data(0x000C,0x0000);
    Write_Command_Data(0x000D,0x080C);
    Write_Command_Data(0x000E,0x2B00);
    Write_Command_Data(0x001E,0x00B7);
    
/*    if (D_ORIENTATION == PORTRAIT)
        Write_Command_Data(0x0001,0x2B3F); // Driver output control - (2B3F)
    else*/
        Write_Command_Data(0x0001,0x293F); // Driver output control - (293F)

    Write_Command_Data(0x0002,0x0600); // LCD drive AC control
    Write_Command_Data(0x0010,0x0000); // Sleep mode

/*    if (D_ORIENTATION == PORTRAIT)
        Write_Command_Data(0x0011,0x6070); // Entry mode - (6070)
    else*/
        Write_Command_Data(0x0011,0x6078); // Entry mode - (6078)
    
    // Compare registers
    Write_Command_Data(0x0005,0x0000);
    Write_Command_Data(0x0006,0x0000);
    
    Write_Command_Data(0x0016,0xEF1C); // Horizontal porch
    Write_Command_Data(0x0017,0x0003); // Vertical porch
    Write_Command_Data(0x0007,0x0033); // Display control
    Write_Command_Data(0x000B,0x0000); // Frame cycle control
    Write_Command_Data(0x000F,0x0000); // Gate scan start position
    
    // Vertical scroll controls
    Write_Command_Data(0x0041,0x0000);
    Write_Command_Data(0x0042,0x0000);
    
    Write_Command_Data(0x0048,0x0000); // First window start
    Write_Command_Data(0x0049,0x013F); // First window end
    Write_Command_Data(0x004A,0x0000); // Second window start
    Write_Command_Data(0x004B,0x0000); // Second window end
    Write_Command_Data(0x0044,0xEF00); // Horizontal RAM address position
    Write_Command_Data(0x0045,0x0000); // Vertical RAM address start position
    Write_Command_Data(0x0046,0x013F); // Vertical RAM address end position
    
    // Gamma controls
    Write_Command_Data(0x0030,0x0707);
    Write_Command_Data(0x0031,0x0204);
    Write_Command_Data(0x0032,0x0204);
    Write_Command_Data(0x0033,0x0502);
    Write_Command_Data(0x0034,0x0507);
    Write_Command_Data(0x0035,0x0204);
    Write_Command_Data(0x0036,0x0204);
    Write_Command_Data(0x0037,0x0502);
    Write_Command_Data(0x003A,0x0302);
    Write_Command_Data(0x003B,0x0302);
    
    // RAM write data mask
    Write_Command_Data(0x0023,0x0000);
    Write_Command_Data(0x0024,0x0000);
    
    Write_Command_Data(0x0025,0x8000); // Frame frequency
    Write_Command_Data(0x004F,0x0000); // GDDRAM Y address counter
    Write_Command_Data(0x004E,0x0000); // GDDRAM X address counter
    
    Write_Command(0x0022);

    bcm2835_gpio_write(TFT_CS,1);

    TFT_Fill(Black);

    TFT_Text((char*)"VDG UT-300 v1.6a\0",0,46,8,White,Black);
    TFT_Text((char*)"TFT GLCD - SSD1289\0",10,46,8,White,Black);
    TFT_Text((char*)"172KB VRAM - 64K Colors\0",20,46,8,White,Black);
    TFT_Text((char*)"www.utilityinf.com.br\0",30,46,8,Red,Black);
    
    Delays(2);

    TFT_Fill(Black);    

    pbytecountw = 0x00;
    pbytecountr = 0x00;       
    preadywr = 0x00;
    preadyrd = 0x00;
    vxcur = 0;
    vycur = 0;
    vcur = 0;
    vcurcont = 0;
    icount = 0;
    icountp = 0;
    keyvxx = 0;
    keyvyy = 0;
    pshowkeyontouch = 0;
    pkeyativo = 0;
    preadycs = 0x00;
    vkeyteste = 0;
    vtimetec = 0;
}

//-------------------------------------------------------------------
unsigned int Set_Color(unsigned int R,unsigned int G,unsigned int B)
{
  unsigned int temp;
  temp = ((R & 0xF8) | (G >> 5)) & 0x00FF;
  G = (G & 0x1C);
  temp |= ((G << 3) | (B >>3)) & 0xFF00;
  return temp;
}

//----------------------------------------------------------------
// Advanced Graphic Functions
//-------------------------------------------------------------------
void TFT_Fill(unsigned int color)
{
  unsigned int i,j;
  bcm2835_gpio_write(TFT_CS,0);
  TFT_Set_Address(0,0,x_max,y_max);
  Write_Data(color);
  for(i = 0; i <= y_max; i++)
  {
    for(j = 0; j <= x_max; j++)
    {
      pBaseVideoMem[0][i][j] = color;
	  	bcm2835_gpio_write(TFT_WR,0);
	  	Delayns(15);
	  	bcm2835_gpio_write(TFT_WR,1);
    }
  }
  bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
void TFT_Box(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color)
{
  unsigned int  i,j;
  bcm2835_gpio_write(TFT_CS,0);
  TFT_Set_Address(X1,Y1,X2,Y2);
  Write_Data(color);
  for(i = Y1; i <= Y2; i++)
  {
    for(j = X1; j <= X2; j++)
    {
      pBaseVideoMem[0][i][j] = color;
	  	bcm2835_gpio_write(TFT_WR,0);
	  	Delayns(15);
	  	bcm2835_gpio_write(TFT_WR,1);
    }
  }
  bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
void TFT_Dot(unsigned int x,unsigned int y,unsigned int color)
{
  bcm2835_gpio_write(TFT_CS,0);
  TFT_Set_Address(x,y,x,y);
  Write_Data(color);
  bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
void TFT_Line(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color)
{
unsigned int x,y,addx,addy,dx,dy;
long P;
unsigned int i;

  dx = fabs(X2-X1);
  dy = fabs(Y2-Y1);
  x = X1;
  y = Y1;

   if(X1 > X2)
   {
       addx = -1;
   }
   else
   {
       addx = 1;
   }

   if(Y1 > Y2)
   {
      addy = -1;
   }
   else
   {
      addy = 1;
   }


 if(dx >= dy)
 {

  P = (2*dy) - dx;

   for(i = 1; i <= (dx +1); i++)
   {

     TFT_Dot(x,y,color);

     if(P < 0)
     {
         P = P + (2*dy);
         x = (x + addx);
     }
     else
     {
        P = P+(2*dy) - (2*dx);
        x = x + addx;
        y = y + addy;
     }
    }
  }
  else
  {
    P = (2*dx) - dy;

    for(i = 1; i <= (dy +1); i++)
    {

     TFT_Dot(x,y,color);

     if(P<0)
     {
       P = P + (2*dx);
       y = y + addy;
     }
     else
     {
        P = P + (2*dx) - (2*dy);
        x = x + addx;
        y = y + addy;
     }
    }
   }
}

//-------------------------------------------------------------------
void TFT_H_Line(char X1,char X2,unsigned int y_pos,unsigned int color)
{
    TFT_Box(X1,y_pos,X2,y_pos+1,color);
}

//-------------------------------------------------------------------
void TFT_V_Line(unsigned int Y1,unsigned int Y2,char x_pos,unsigned int color)
{
    TFT_Box(x_pos,Y1,x_pos+1,Y2,color);
}

//-------------------------------------------------------------------
void TFT_Rectangle(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color)
{
    TFT_H_Line(X1,X2,Y1,color);
    TFT_H_Line(X1,X2,Y2,color);
    TFT_V_Line(Y1,Y2,X1,color);
    TFT_V_Line(Y1,Y2,X2,color);
}

//-------------------------------------------------------------------
void TFT_Circle(unsigned int x,unsigned int y,char radius,char fill,unsigned int color)
{
 int a_,b_,P;
 a_ = 0;
 b_ = radius;
 P = 1 - radius;
 while (a_ <= b_)
 {
    if(fill == 1)
    {
         TFT_Box(x-a_,y-b_,x+a_,y+b_,color);
         TFT_Box(x-b_,y-a_,x+b_,y+a_,color);
    }
    else
    {
         TFT_Dot(a_+x, b_+y, color);
         TFT_Dot(b_+x, a_+y, color);
         TFT_Dot(x-a_, b_+y, color);
         TFT_Dot(x-b_, a_+y, color);
         TFT_Dot(b_+x, y-a_, color);
         TFT_Dot(a_+x, y-b_, color);
         TFT_Dot(x-a_, y-b_, color);
         TFT_Dot(x-b_, y-a_, color);
    }
    if (P < 0 )
    {
        P = (P + 3) + (2* a_);
        a_ ++;
    }
    else
    {
        P = (P + 5) + (2* (a_ - b_));
        a_ ++;
        b_ --;
    }
  }
}

//-------------------------------------------------------------------
void TFT_Char(char C,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor)
{
	unsigned char *PtrFont;
	unsigned int CPtrFont, Cint;
	unsigned char font8x8[8], uuc;
	unsigned int font16X16[16];
	unsigned char k,i,print1,print2;
	unsigned int print3,print4;
	
	if(DimFont == 8)
	{
		   uuc = C;
	     Cint = uuc;
	     CPtrFont = (Cint - 32) * 8;
	     PtrFont = FONT_8x8 + CPtrFont;
	
	     for(k = 0; k <= 7; k++) {
	         font8x8[k] = *PtrFont++;
	     }
   
       bcm2835_gpio_write(TFT_CS,0);
	     TFT_Set_Address(x,y,x+7,y+7);
	     for(i = 0; i <= 7; i++)
	     {
	       for(k = 0; k <= 7; k++)
	       {
	          print1 = (font8x8[i] & 0x80);
	          print2 = print1 >>7;
	          if(print2 == 1)
	          {
               pBaseVideoAddrY = (x + i);
               pBaseVideoAddrX = (y + k);
	             Write_Data(Fcolor);
	          }
	          else
	          {
               pBaseVideoAddrY = (x + i);
               pBaseVideoAddrX = (y + k);
	             Write_Data(Bcolor);
	          }
	          font8x8[i] = font8x8[i] << 1;
	       }
	     }

       bcm2835_gpio_write(TFT_CS,1);
	}
	
	else if(DimFont == 16)
	{
		   uuc = C;
	     Cint = uuc;
       CPtrFont = (Cint - 32) * 32;
	     PtrFont = FONT_16X16 + CPtrFont;
	
	     for(k = 0; k <= 15; k++)
	     {
	       font16X16[k] = *PtrFont++;
	       font16X16[k] = (font16X16[k] << 8);
	       font16X16[k] = font16X16[k] + *PtrFont++;
	     }
	
       bcm2835_gpio_write(TFT_CS,0);

	     TFT_Set_Address(x,y,x+15,y+15);
	     
       for(i = 0; i <= 15; i++)
	     {
	       for(k = 0; k <= 15; k++)
	       {
	         print3 = (font16X16[i] & 0x8000);
	         print4 = print3 >>15;
	
	         if(print4 == 1)
	         {
              pBaseVideoAddrY = (x + i);
              pBaseVideoAddrX = (y + k);
	            Write_Data(Fcolor);
	         }
	         else
	         {
              pBaseVideoAddrY = (x + i);
              pBaseVideoAddrX = (y + k);
	            Write_Data(Bcolor);
	         }
	
	         font16X16[i] = font16X16[i] << 1;
	       }
	     }

       bcm2835_gpio_write(TFT_CS,0);
	}
}

//-------------------------------------------------------------------
void TFT_Text(char* S,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor)
{
    while (*S)
    {
        TFT_Char(*S++,x,y,DimFont,Fcolor,Bcolor);
        
        y = y + DimFont;
    }
    vxcur = x;
    vycur = y;
}

//-------------------------------------------------------------------
void TFT_Image(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y,unsigned int *picture) {
    unsigned int x, y;

    bcm2835_gpio_write(TFT_CS,0);
    TFT_Set_Address(pos_x, pos_y, pos_x + dim_y - 1, pos_y + dim_x - 1);
    for(y = pos_y; y < (pos_y + dim_x); y++ ) {
	    for(x = pos_x; x < (pos_x + dim_y); x++ ) {
            pBaseVideoAddrX = y;
            pBaseVideoAddrY = x;
            Write_Data(*picture++);
        }
    }
    bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
void TFT_InvertRect(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y) {
/*    unsigned int iy,xf,yf;

    yf = pos_y + dim_x;
    xf = pos_x + dim_y;

    for (ix = pos_x; ix <= xf; ix++) {
	    TFT_Set_Address(ix,pos_y,ix,yf);

		// read actual position
	    TFT_DP_Lo0_Direction = 0xFF;
	    TFT_DP_Lo1_Direction = 0xFF;
	    TFT_DP_Lo_Direction = 0xFF;
	    TFT_DP_Hi_Direction = 0xFF;
		TFT_SCK_Dir = 0x00;
		TFT_SDI_Dir = 0x01;

        // dummy read
        Read_Data();	

        // real read
	    for (iy = pos_y; iy <= yf; iy++) {
	        pvideo[iy] = Read_Data();
	    }

        // write in the new position inverted
	    TFT_DP_Lo0_Direction = 0x00;
	    TFT_DP_Lo1_Direction = 0x00;
	    TFT_DP_Lo_Direction = 0x00;
	    TFT_DP_Hi_Direction = 0x00;
		TFT_SCK_Dir = 0x00;
		TFT_SDI_Dir = 0x01;

	    TFT_Set_Address(ix,pos_y,ix,yf);
	    for (iy = pos_y; iy <= yf; iy++) {
	        Write_Data(~pvideo[iy]);
	    }
    }*/
}

//-------------------------------------------------------------------
void TFT_Scroll(unsigned char qtdlin) 
{
    unsigned int iy,xo,xd,xf,yf;

    xd = 0;
    xo = qtdlin;
    yf = y_max;
    xf = x_max;

    bcm2835_gpio_write(TFT_CS,0);

    for (ix = xo; ix <= xf; ix++) 
    {
  	    for (iy = 0; iy <= yf; iy++) 
        {
            pvideo[iy] = pBaseVideoMem[0][iy][ix];
  	    }

        xd = ix - qtdlin;
	      TFT_Set_Address(xd,0,xd,yf);
  	    for (iy = 0; iy <= yf; iy++) 
        {
            pBaseVideoAddrX = iy;
            pBaseVideoAddrY = xd;
  	        Write_Data(pvideo[iy]);
  	    }
    }

    xd++;
    TFT_Box(xd,0,xf,yf,bcor);

    bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
// 11 Paginas disponiveis, usaveis de 1 a 10
// Pagina 0 é a que esta sendo mostrada na tela
//-------------------------------------------------------------------
void TFT_SaveScreen(unsigned int pPos, unsigned int x, unsigned int y, unsigned int width, unsigned int height) 
{
    unsigned int temp;
    unsigned int vxxw, vyyw;

    vxxw = y + height;
    vyyw = x + width;

    for (ix = y; ix <= vxxw; ix++) 
    {
  	    for (iy = x; iy <= vyyw; iy++) 
        {
            temp = pBaseVideoMem[0][iy][ix];
            pBaseVideoMem[pPos][iy][ix] = temp;
  	    }
    }
}

//-------------------------------------------------------------------
// 11 Paginas disponiveis, usaveis de 1 a 10
// Pagina 0 é a que esta sendo mostrada na tela
//-------------------------------------------------------------------
void TFT_RestoreScreen(unsigned int pPos, unsigned int x, unsigned int y, unsigned int width, unsigned int height) 
{
    unsigned int data, vxxw, vyyw;

    vxxw = y + height;
    vyyw = x + width;

    bcm2835_gpio_write(TFT_CS,0);

    for (ix = y; ix <= vxxw; ix++) 
    {
        TFT_Set_Address(ix,x,ix,vyyw);
        for (iy = x; iy <= vyyw; iy++) 
        {
            data = pBaseVideoMem[pPos][iy][ix];
            pBaseVideoAddrX = iy;
            pBaseVideoAddrY = ix;
            Write_Data(data);
        }
    }
    
    bcm2835_gpio_write(TFT_CS,1);
}

//-------------------------------------------------------------------
// General Functions
//-------------------------------------------------------------------
int fabs(int val)  
{ 
     return (val<0 ? (-val) : val);
} 
 
//-------------------------------------------------------------------
// Keyboard Functions
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void ReadKeyMatrix(unsigned char vini, unsigned char vlin, unsigned char vqtd) {
	unsigned char vkey;
	for (vkey = 0; vkey <= vqtd; vkey++) 
		vlaytec[vlin][vkey] = KEY_MOD[vini + vkey];
}

//-------------------------------------------------------------------
void HideKeyboard(unsigned int xposini, unsigned int yposini) {
	pkeyativo = 0x00;

	// Restore old screen
	TFT_RestoreScreen(0x01, xposini, yposini, 221, 72);
}

//-------------------------------------------------------------------
void ShowKeyboard(unsigned int xposini, unsigned int yposini, unsigned char vnewkey) {
	unsigned int tl, tc, tx, ty;
	unsigned char vtecbk, vtecshift[4], vtecfim[4] = {0,0,0,0}, vteccaps[4], vtam = 0;

	if (!pkeyativo) {
		// Save actual position
		TFT_SaveScreen(0x01, xposini, yposini, 221, 72);
		
		pkeyativo = 0x01;
	}

	vtecbk = 130;

	switch (vtipofimkey) {
	  case 0x00:
	    vtecfim[0] = 'e';
	    vtecfim[1] = 'n';
	    vtecfim[2] = 'd';
		vtam = 8;
	    break;
	  case 0x01:
	    vtecfim[0] = ' ';
	    vtecfim[1] = 131;   // TECLA ENTER
	    vtecfim[2] = ' ';
		vtam = 16;
	    break;
	}
	  
	if (vcapson) {
	  vteccaps[0] = 'C';
	  vteccaps[1] = 'A';
	  vteccaps[2] = 'P';
	}
	else {
	  vteccaps[0] = 'c';
	  vteccaps[1] = 'a';
	  vteccaps[2] = 'p';
	}

    vtecshift[0] = 'A';
    vtecshift[1] = 'B';
    vtecshift[2] = 'C';

    if (vtipokey == 0) {       // Teclado Com Letras
      vtecshift[0] = '1';
      vtecshift[1] = '2';
      vtecshift[2] = '3';
	  ReadKeyMatrix(0, 0, 9);		// QWERTYUIOP
	  ReadKeyMatrix(10, 1, 9);		// ASDFGHJKL.
	  ReadKeyMatrix(20, 2, 9);		// ZXCVBNM
    }
    else if (vtipokey == 1) {  // Teclado com Numeros e Simbolos
      vtecshift[0] = '#';
      vtecshift[1] = '+';
      vtecshift[2] = '=';
	  ReadKeyMatrix(32, 0, 9);	    // 1234567890
	  ReadKeyMatrix(42, 1, 9);		// !@#$%|&*()
	  ReadKeyMatrix(52, 2, 9);		// ,.;:?<>
    }                          
    else if (vtipokey == 2) {  // Teclado Com Simbolos
      vtecshift[0] = 'A';
      vtecshift[1] = 'B';
      vtecshift[2] = 'C';
	  ReadKeyMatrix(64, 0, 9);		// []{}`\_+-=
	  ReadKeyMatrix(74, 1, 9);		// /         
	  ReadKeyMatrix(84, 2, 9);		//        
    }

	if (!vfirstshowkey || (vfirstshowkey && vtipokey)) {
		if (vnewkey) {      
		  // Clear Keyboard Area
		  TFT_Box(yposini, xposini, yposini + 72, xposini + 221, Black);

		  // Draw Keyboard
		  if (vtipofimkey)
	          TFT_Char('X', yposini, xposini + 206, 16 , White, Red);		

	      tx = 0;
	      ty = yposini;
	      
	      for (tl = 0; tl <= 2; tl++) {
	        for (tc = 0; tc <= 9; tc++) {
	          if (tl == 0) 
	            tx = xposini + 5 + ((20 * tc) + 1);
	          else if (tl == 1)  
	            tx = xposini + 7 + ((20 * tc) + 1);
	          else if (tl == 2) 
	            tx = xposini + 5 + ((19 * tc) + 1);
	
	          ty = yposini + ((17 * tl) + 1);
	          
	          vlayx[tl][tc] = (tx - 2);        
	          tx++; 
	          ty++;
	  
	          if (tl <= 1)           
	            TFT_Char(vlaytec[tl][tc],ty,tx,16,Black,White);
	          else if (tl == 2 && tc >= 2 && tc <= 8)           
	            TFT_Char(vlaytec[2][tc - 2],ty,tx,16,Black,White);
	          else if (tl == 2 && tc == 9)           
	            TFT_Char(vtecbk,ty,tx,16,Black,White);
	        }
	      }
		}
	
	    TFT_Box(yposini + 54,xposini + 40,yposini + 70,xposini + 186,White);  // Tecla Espaço
	
	    TFT_Box(yposini + 36,xposini + 7,yposini + 52,xposini + 37,White);  					// Tecla Caps
	    TFT_Char(vteccaps[0],yposini + 40,xposini + 11,8,Black,White);		
	    TFT_Char(vteccaps[1],yposini + 40,xposini + 19,8,Black,White);
	    TFT_Char(vteccaps[2],yposini + 40,xposini + 27,8,Black,White);
	
	    TFT_Box(yposini + 54,xposini + 7,yposini + 70,xposini + 37,White);  					// Tecla 123
	    TFT_Char(vtecshift[0],yposini + 58,xposini + 11,8,Black,White);		
	    TFT_Char(vtecshift[1],yposini + 58,xposini + 19,8,Black,White);
	    TFT_Char(vtecshift[2],yposini + 58,xposini + 27,8,Black,White);
	
	    TFT_Box(yposini + 54,xposini + 189,yposini + 70,xposini + 219,White);  					// Tecla Fim
	    TFT_Char(vtecfim[0],yposini + 54,xposini + 193,8,Black,White);	
	    TFT_Char(vtecfim[1],yposini + 54,xposini + 201,vtam,Black,White);
	    TFT_Char(vtecfim[2],yposini + 54,xposini + 209,8,Black,White);
	}
	else
		vfirstshowkey = 0;		
}

void ReturnKeyboard(unsigned int xposini, unsigned int yposini, unsigned int vpostx, unsigned int vposty) {
	unsigned char tc;
	unsigned int vpy = 0, vpx = 0;
  ptec[1] = 0x00;

/*  char* sqtdtam = '\0';
    itoa(vpostx, sqtdtam, 10);
    TFT_Text("       ",100,220,8,White,Blue);
    TFT_Text(sqtdtam,100,220,8,White,Blue);
    itoa(vposty, sqtdtam, 10);
    TFT_Text("       ",200,220,8,White,Red);
    TFT_Text(sqtdtam,200,220,8,White,Red);*/

    //--- VERIFICA A POSIÇÃO PRESSIONADA
    if (vposty >= yposini && vposty <= (yposini + 17)) {    
      vpy = yposini + 1;              		  
      if (vpostx >= (xposini + 205) && vpostx <= (xposini + 223)) {				  // X - Fechar Teclado
  		  if (vtipofimkey) {
  		      vpx = xposini + 205;
  	      	ptec[1] = 0x01;
  			    ptec[2] = 3;	// Botão Fechar Teclado
  	        tc = 10;
  		  }
    }
	  else {
	      for (tc = 0; tc <= 9; tc++) {
	        if (vpostx >= vlayx[0][tc] && vpostx <= (vlayx[0][tc] + 19)) {		  // QWERT
		        vpx = vlayx[0][tc] + 2;
	        	ptec[1] = 0x01;

				if (vcapson || vtipokey > 0)
					ptec[2] = vlaytec[0][tc];
				else
					ptec[2] = (vlaytec[0][tc] + 32);

	          	tc = 10;
	        }
	      }
	  }
    } 
    else if (vposty >= (yposini + 18) && vposty <= (yposini + 34)) {             // ASDFG 
      vpy = yposini + 18;              		  
      for (tc = 0; tc <= 9; tc++) {
        if (vpostx >= vlayx[1][tc] && vpostx <= (vlayx[1][tc] + 19)) {
	        vpx = vlayx[1][tc] + 2;
        	ptec[1] = 0x01;

			if (vcapson || vtipokey > 0 || tc == 9)
				ptec[2] = vlaytec[1][tc];
			else
				ptec[2] = (vlaytec[1][tc] + 32);

          	tc = 10;
        }
      }
    } 
    else if (vposty >= (yposini + 35) && vposty <= (yposini + 52)) {              
      vpy = yposini + 35;              		  
      if (vpostx >= (xposini + 7) && vpostx <= (xposini + 37)) {				  // Caps
          vpx = xposini + 7;
      	  ptec[1] = 0x01;
		  ptec[2] = 2;	// Tecla CAPS
          tc = 10;
      }
	  else {
	      for (tc = 2; tc <= 9; tc++) {											  // ZXCVB e Backspace
	        if (vpostx >= vlayx[2][tc] && vpostx <= (vlayx[2][tc] + 19)) {		
		        vpx = vlayx[2][tc] + 2;
	        	ptec[1] = 0x01;

				if (tc < 9) {
					if (vcapson || vtipokey > 0)
						ptec[2] = vlaytec[2][tc - 2];
					else 
						ptec[2] = (vlaytec[2][tc - 2] + 32);
				}
				else
					ptec[2] = 8;

	          	tc = 10;
	        }
	      }
	  }
	}
    else if (vposty >= (yposini + 54) && vposty <= (yposini + 70)) {              
      vpy = yposini + 54;              		  
      if (vpostx >= (xposini + 40) && vpostx <= (xposini + 186)) {				  // Espaço
          vpx = xposini + 40;
      	  ptec[1] = 0x01;
		  ptec[2] = 32;
          tc = 10;
      }
      else if (vpostx >= (xposini + 189) && vpostx <= (xposini + 219)) {	 	  // Enter
          vpx = xposini + 201;
          ptec[1] = 0x01;
		  ptec[2] = 13;
          tc = 10;
      }
      else if (vpostx >= (xposini + 7) && vpostx <= (xposini + 37)) {			  // ABC -> 123 -> #+=
          vpx = xposini + 7;
          ptec[1] = 0x01;
		  ptec[2] = 1;	// Tecla 123 ou ABC
          tc = 10;
      }
    }

	if (ptec[1] && vpx != 0 && vpy != 0) {
		TFT_InvertRect(vpy, vpx, 16, 16);
		Delayms(200);	// +/- 200mS
		TFT_InvertRect(vpy, vpx, 16, 16);
	}
}
