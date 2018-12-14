#ifndef __LCD_VDG__
#define __LCD_VDG__

#define PORTRAIT 0
#define LANDSCAPE 1

#define Black 0x0000
#define Red 0xF800
#define Green 0x07E0
#define Blue 0x001F
#define White 0xFFFF
#define Purple 0x780F
#define Yellow 0xFFE0
#define Cyan 0x07FF
#define d_gray 0x03EF
#define l_gray 0xC618
#define Lime 0x87E0
#define Aqua 0x5D1C
#define Magenta 0xF81F
#define Orange 0xFCA0
#define Pink 0xF97F
#define Brown 0x8200
#define Violet 0x9199
#define Silver 0xA510
#define Gold 0xA508
#define Navy 0x000F
#define Maroon 0x7800
#define Olive 0x7BE0

#define TFT_DC RPI_V2_GPIO_P1_15	
#define TFT_RST RPI_V2_GPIO_P1_16
#define TFT_CS RPI_V2_GPIO_P1_12	
#define TFT_WR RPI_V2_GPIO_P1_13
#define TFT_RD RPI_V2_GPIO_P1_11	
#define TFT_D0 RPI_V2_GPIO_P1_29	
#define TFT_D1 RPI_V2_GPIO_P1_31	
#define TFT_D2 RPI_V2_GPIO_P1_33	
#define TFT_D3 RPI_V2_GPIO_P1_35	
#define TFT_D4 RPI_V2_GPIO_P1_37	
#define TFT_D5 RPI_V2_GPIO_P1_36	
#define TFT_D6 RPI_V2_GPIO_P1_38	
#define TFT_D7 RPI_V2_GPIO_P1_40	
#define LS373_LE RPI_V2_GPIO_P1_18

typedef struct
{
  int x;
  int y;
} Coordinate;

class CLcdVdg
{
public:
    CLcdVdg (void);
    ~CLcdVdg (void);

	boolean Initialize (void);

	int TCHVerif(unsigned char pRetAds, Coordinate screen_cal);
	int commVDG(unsigned char *vparam);

	//----------------------------------------------------------------
	// LCDG Advanced Graphic Functions
	//----------------------------------------------------------------
	void TFT_Dot(unsigned int x,unsigned int y,unsigned int color);
	void TFT_Fill(unsigned int color);
	void TFT_Box(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
	void TFT_Line(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
	void TFT_H_Line(char X1,char X2,unsigned int y_pos,unsigned int color);
	void TFT_V_Line(unsigned int Y1,unsigned int Y2,char x_pos,unsigned int color);
	void TFT_Rectangle(unsigned int X1,unsigned int Y1,unsigned int X2,unsigned int Y2,unsigned int color);
	void TFT_Circle(unsigned int x,unsigned int y,char radius,char fill,unsigned int color);
	void TFT_WriteChar(char C,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor);
	void TFT_Char(char C,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor);
	void TFT_Text(char* S,unsigned int x,unsigned int y,char DimFont,unsigned int Fcolor,unsigned int Bcolor);
	void TFT_Load_Image(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y,unsigned int *picture);
	void TFT_Show_Image(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y);
	void TFT_Show_Image(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y,unsigned int *picture);
	void TFT_InvertRect(unsigned int pos_x,unsigned int pos_y,unsigned int dim_x,unsigned int dim_y);
	void TFT_Scroll(unsigned char qtdlin, unsigned int pColor);
	void TFT_Scroll(unsigned char qtdlin);
	void TFT_SaveScreen(unsigned int pPos, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	void TFT_RestoreScreen(unsigned int pPos, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

private:
	static CLcdVdg *s_pThis;
	
	unsigned char pInit = 0;
    unsigned int pOldData = 0;
	volatile unsigned int x_max = 239;
	volatile unsigned int y_max = 319;
	volatile unsigned char pmode = 1;	// 1 - lcd, 2 - vga
	volatile unsigned int pvideo[576];

	unsigned char vmove;
	unsigned int long vtimetec;
	unsigned char pDataRdy;

	unsigned int pBaseVideoAddrX = 0;
	unsigned int pBaseVideoAddrY = 0;

	volatile unsigned char pbytecountw;
	volatile unsigned char pbytecountr;
	volatile unsigned char preadywr;
	volatile unsigned char preadyrd;
	volatile unsigned char preadycs;

	volatile unsigned char keyBuffer[16];
	volatile unsigned char *keyPntr=keyBuffer;
	volatile unsigned char *keysend=keyBuffer;
	volatile unsigned char *keysend2=keyBuffer;
	volatile unsigned int pxxxreturn[16];
	volatile unsigned int pyyyreturn[16];
	volatile unsigned char ptec[3];
	volatile unsigned char pkeyativo = 0;
	volatile unsigned char pshowkeyontouch = 0;
	volatile unsigned char vtipofimkey = 1;	// 0 - End, 1 - Enter
	volatile unsigned char vcapson = 0;		// 0 - Minusc, 1 - Maiusc
	volatile unsigned char vtipokey = 0;	// 0 - Letra, 1 - Numerico, 2 - Simbolos
	volatile unsigned char vkeyrep = 0;
	unsigned char vkeyteste = 0;
	char irqTchOn = 0;

	unsigned int vxx, vyy, keyvxx, keyvyy, vxxf, vyyf, fcor = 65535, bcor = 0;
	unsigned int fcorcur = 65535, bcorcur = 0, fcorgraf = 0;
	unsigned int vxcur, vycur;
	unsigned int long vcurcont = 0;
	unsigned char dfont, cchar;
	unsigned char vcur = 0;
	unsigned char D_ORIENTATION;
	unsigned int iz, ix, iy, pconv1, pconv2;

	volatile unsigned char vlaytec[3][12];
	volatile unsigned int vlayx[3][12];
	volatile unsigned char vfirstshowkey = 0;
	volatile unsigned int icount = 0, icountp = 0;

	volatile unsigned char KBD_DATARDY = 0x00;	// Temporario, até ver se vai ser interrupcao ou outra coisa

	Coordinate screen_cal;

private:
	//----------------------------------------------------------------
	// General Functions
	//----------------------------------------------------------------
	int fabs(int val);  

	//----------------------------------------------------------------
	// LCDG Basic Graphic Functions
	//----------------------------------------------------------------
	void Write_Bytes_GPIO(unsigned int wByte);
	void Write_Command(unsigned int wcommand);
	void Write_Data(unsigned int wdata);
	unsigned int Read_Data(void);
	void Write_Command_Data(unsigned int wcommand,unsigned int Wdata);
	void TFT_Set_Address(unsigned int PX1,unsigned int PY1,unsigned int PX2,unsigned int PY2);
	void TFT_Init(void);
	unsigned int Set_Color(unsigned int R,unsigned int G,unsigned int B);

	//----------------------------------------------------------------
	// Keyboard Functions
	//----------------------------------------------------------------
	void ReadKeyMatrix(unsigned char vini, unsigned char vlin, unsigned char vqtd);
	void ShowKeyboard(unsigned int xposini, unsigned int yposini, unsigned char vnewkey);
	void HideKeyboard(unsigned int xposini, unsigned int yposini);
	void ReturnKeyboard(unsigned int xposini, unsigned int yposini, unsigned int vpostx, unsigned int vposty);

	void lcd_vdg_Delayns(long pTime);
	void lcd_vdg_Delayms(int pTime);
};

#endif
