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
	
	void locatexy(unsigned int pposx, unsigned int ppoxy);
	void writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor);
	void writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor);
	void SetDot(unsigned int x, unsigned int y, unsigned int color);
	void FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor);
	void DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);
	void DrawRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight, unsigned int color);
	void DrawRoundRect(void);
	void DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor);
	void PutImage(unsigned char* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);
	void RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);

	static CScrTft *Get (void);

private:
	CInterruptSystem *m_pInterruptSystem;
	CLcdVdg *p_mLcdVdg;
	CLcdTch *p_mLcdTch;

	static CScrTft *s_pThis;

	unsigned int  voutput; // 0 - LCD (16x2), 1 - LCD Grafico (320 x 240), 2 - VGA (somente modo texto)
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
	unsigned int vparam[29]; //  29 Parameters

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

#endif
