#ifndef __SCRTFTGUI__
#define __SCRTFTGUI__

typedef struct sWindowScr
{
	unsigned int   wPosX;
	unsigned int   wPosY;
	unsigned int   wWidth;
	unsigned int   wHeight;
	unsigned int   wColorf;
	unsigned int   wColorb;
	char          *wTitle = new char[255];
	unsigned char  wButtons;

} WINDOWSCR;

typedef struct sWindowElement
{
	unsigned char  weType;
	unsigned int   wePosX;
	unsigned int   wePosY;
	unsigned int   weWidth;
	unsigned int   weHeight;
	unsigned int   weColorf;
	unsigned int   weColorb;
	long           weOpc[255];
	char 		  *weString = new char[255];
	char 		  *weReturn = new char[255]; 
} WINDOWELEMENT;

class CWindow
{
public:
    CWindow (CScrTft *mScrTft, char* ptitle, unsigned int x1, unsigned int y1, unsigned int pwidth, unsigned int pheight, unsigned int pcolorf, unsigned int pcolorb);
    ~CWindow (void);

	unsigned char addElement(unsigned char pElement, unsigned int pPosX, unsigned int pPosY, unsigned int pWidth, unsigned int pHeight, unsigned int pcolorf, unsigned int pcolorb, unsigned char* pString, int pnumopc, ...);
	unsigned char run(unsigned int ptime, int pnumbuttons, ...);
	unsigned char* GetReturnElement(unsigned char pElement);

	void radioset(unsigned char pElement, unsigned char vtipo);
	void togglebox(unsigned char pElement, unsigned char vtipo);
	void fillin(unsigned char pElement, unsigned char vtipo);
	void spin(unsigned char pElement, unsigned char vtipo);
	void button(unsigned char pElement, unsigned char vTipo);
	void slider(unsigned char pElement, unsigned char vtipo);
	void combobox(unsigned char pElement, unsigned char vtipo);
	void editor(unsigned char pElement, unsigned char vtipo);

	static CWindow *Get (void);

private:
	CScrTft *m_pScrTft;

	static CWindow *s_pThis;

	WINDOWSCR vWindow;
	WINDOWELEMENT vElements[255];
	unsigned char vCountElements;

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
	unsigned int vpostx, vposty;

	unsigned char paramVDG[255];
    unsigned int vbytepic = 0, vbytevdg;

    unsigned char pKeyboardType;
};

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

#define GUIFILLIN    0x01
#define GUIRADIOSET  0x02
#define GUITOGGLEBOX 0x03
#define GUICOMBOBOX  0x04
#define GUIEDITOR    0x05
#define GUITEXT      0x06
#define GUIBUTTON    0x07
#define GUISPIN      0x08
#define GUISLIDER    0x09

#endif
