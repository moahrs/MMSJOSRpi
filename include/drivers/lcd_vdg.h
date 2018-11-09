#define PORTRAIT 0
#define LANDSCAPE 1

volatile unsigned int x_max = 239;
volatile unsigned int y_max = 319;

#define TFT_DC RPI_V2_GPIO_P1_15	
#define TFT_RST RPI_V2_GPIO_P1_16
#define TFT_CS RPI_V2_GPIO_P1_12	
#define TFT_WR RPI_V2_GPIO_P1_10
#define TFT_RD RPI_V2_GPIO_P1_08	
#define TFT_D0 RPI_V2_GPIO_P1_29	
#define TFT_D1 RPI_V2_GPIO_P1_31	
#define TFT_D2 RPI_V2_GPIO_P1_33	
#define TFT_D3 RPI_V2_GPIO_P1_35	
#define TFT_D4 RPI_V2_GPIO_P1_37	
#define TFT_D5 RPI_V2_GPIO_P1_36	
#define TFT_D6 RPI_V2_GPIO_P1_38	
#define TFT_D7 RPI_V2_GPIO_P1_40	
#define LS373_LE RPI_V2_GPIO_P1_18

volatile unsigned char pmode = 1;	// 1 - lcd, 2 - vga
volatile unsigned int pvideo[576];

unsigned char vmove;
unsigned int long vtimetec;
unsigned char pDataRdy;

unsigned int* pBaseVideoMem = (unsigned int*)0x1FC00000; // 4MB Video Memory 0x1FC00000 or 0x5FC00000
unsigned int pBaseVideoAddr;

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

