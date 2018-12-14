#ifndef __SCRTFT__
#define __SCRTFT__

#include <circle/device.h>
#include <ScreenTFT/lcd_vdg.h>
#include <ScreenTFT/lcd_tch.h>

class CScrTft : public CDevice
{
public:
    CScrTft (CInterruptSystem *mInterrupt);
    ~CScrTft (void);

	boolean Initialize (void);

	unsigned GetOutput (void) const;
	void SetOutput (unsigned pOutput);
	void SetTypeKeyboard(unsigned pTypeKey);

	unsigned GetWidth (void) const;
	unsigned GetHeight (void) const;
	unsigned GetColumns (void) const;
	unsigned GetRows (void) const;
	unsigned GetRow (void) const;
	void SetRow (unsigned pRow);
	unsigned GetColumn (void) const;
	void SetColumn (unsigned pCol);
	unsigned GetColorForeground (void) const;
	void SetColorForeground (unsigned pColorF);
	unsigned GetColorBackground (void) const;
	void SetColorBackground (unsigned pColorB);

	void clearScr();
	void clearScr(unsigned int pcolor);
	int Write (const void *pBuffer, unsigned nCount);
	void writestr(char *msgs);
	void writechar(int c);
	void locate(unsigned char pcol, unsigned char plin, unsigned char pcur);
	void writes(char *msgs, unsigned int pcolor, unsigned int pbcolor);
	void writec(unsigned char pbyte, unsigned char ptipo);
	void writec(unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor, unsigned char ptipo);
	void funcKey(unsigned char vambiente, unsigned char vshow, unsigned char venter, unsigned char vtipo, unsigned int x, unsigned int y);
	void ativaCursor(void);
	void desativaCursor(void);
	void blinkCursor(void);
	unsigned char verifKey(void);
	unsigned char getKey(void);
	void getPos(unsigned int *pPostX, unsigned int *pPostY);

	void locatexy(unsigned int pposx, unsigned int ppoxy);
	void writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor);
	void writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor);
	void SetDot(unsigned int x, unsigned int y, unsigned int color);
	void FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor);
	void DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);
	void DrawRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight, unsigned int color);
	void DrawRoundRect(void);
	void DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor);
	void PutImage(unsigned int* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight);
	void RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight);

	void VerifyTouchLcd(unsigned char vtipo, unsigned int *ppostx, unsigned int *pposty);
	void showWindow(unsigned char* vparamstrscr, unsigned int *vparamscr);
	void drawButtons(unsigned int xib, unsigned int yib);
	unsigned char waitButton(void);
	unsigned char message(char* bstr, unsigned char bbutton, unsigned int btime);
	void radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
	void togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
	void fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo);
	void showImageBMP(unsigned int posx, unsigned int posy, unsigned int pwidth, unsigned int pheight, unsigned char* pfileImage);

	static CScrTft *Get (void);

private:
	CInterruptSystem *m_pInterruptSystem;
	CLcdVdg *p_mLcdVdg;
	CLcdTch *p_mLcdTch;

	static CScrTft *s_pThis;

	unsigned int  voutput; // 1 - LCD Texto (40 x 24) 2 - LCD Grafico (320 x 240)
	unsigned int  vcorf; // cor padrao de frente
	unsigned int  vcorb; // cor padrao de fundo
	unsigned int  vcol;
	unsigned int  vlin;
	unsigned int  vxmax;
	unsigned int  vymax;
	unsigned int  vxgmax;
	unsigned int  vygmax;
	unsigned int voverx; //  Overlay video texto para janelas
	unsigned int vovery; //  Overlay video texto para janelas
	unsigned int vxmaxold;
	unsigned int vymaxold; 
	unsigned int vcorwb;
	unsigned int vcorwf;
	unsigned int pposx;
	unsigned int pposy;
	unsigned int vpostx;
	unsigned int vposty;
	unsigned int  xpos;
	unsigned int  ypos;
    unsigned char pKeyboardType;
	unsigned int vparam[29]; //  29 Parameters

	unsigned int  viconef; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
	unsigned int  verro;
	unsigned char  next_pos;
	unsigned char vparamstr[255]; //  255 Char Param string
	unsigned char vbbutton;
	unsigned char vkeyopen;
	unsigned char vbytetec;
	unsigned int vbuttonwiny;
	unsigned char vbuttonwin[100];
	unsigned char vpagescr;

	unsigned char paramVDG[255];
    unsigned int vbytepic = 0, vbytevdg;
};

#define DrawVertLine(x1, y1, length, color) FillRect(x1, y1, 0, length, color)
#define DrawHoriLine(x1, y1, length, color) FillRect(x1, y1, length, 0, color)

#define lcdBusy 0x80

#define NOREPOS_CURSOR             0
#define REPOS_CURSOR               1
#define REPOS_CURSOR_ON_CHANGE     2
#define ADD_POS_SCR                3
#define NOADD_POS_SCR              4


#define xlen_lcdg_por 30
#define ylen_lcdg_por 32

#define xlen_lcdg_lan 40
#define ylen_lcdg_lan 24

#define xlen_vga 30
#define ylen_vga 30

#define addline 0x01
#define noaddline 0x00

#define BTNONE      0x00
#define BTOK        0x01
#define BTCANCEL    0x02
#define BTYES       0x04
#define BTNO        0x08
#define BTHELP      0x10
#define BTSTART     0x20
#define BTCLOSE     0x40

#define WINVERT     0x01
#define WINHORI     0x00

#define WINOPER     0x01
#define WINDISP     0x00

#define WHAITTOUCH   0X01
#define NOWHAITTOUCH 0x00

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    u16 bfType;  //specifies the file type
    u32 bfSize;  //specifies the size in bytes of the bitmap file
    u16 bfReserved1;  //reserved; must be 0
    u16 bfReserved2;  //reserved; must be 0
    u32 bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    u32 biSize;  //specifies the number of bytes required by the struct
    u32 biWidth;  //specifies width in pixels
    u32 biHeight;  //species height in pixels
    u16 biPlanes; //specifies the number of color planes, must be 1
    u16 biBitCount; //specifies the number of bit per pixel
    u32 biCompression;//spcifies the type of compression
    u32 biSizeImage;  //size of image in bytes
    u32 biXPelsPerMeter;  //number of pixels per meter in x axis
    u32 biYPelsPerMeter;  //number of pixels per meter in y axis
    u32 biClrUsed;  //number of colors used by th ebitmap
    u32 biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;

#pragma pack(pop)

#endif
