#ifndef __SCRTFT__
#define __SCRTFT__

#include <circle/device.h>

class CScrTft : public CDevice
{
public:
    CScrTft (CLcdVdg *mLcdVdg);
    ~CScrTft (void);

	boolean Initialize (void);

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
	void putS(const char* str);
	int printf(const char *format, ...);
	int vsprintf(char *str, const char *format, va_list listPointer);
	int sprintf(char *str, const char *format, ...);

private:
	CLcdVdg *p_mLcdVdg;
	
	static CScrTft *s_pThis;

	unsigned char paramVDG[255];
    unsigned int vbytepic = 0, vbytevdg;
};

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