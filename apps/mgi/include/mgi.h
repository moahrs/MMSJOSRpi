#ifndef __MGI__
#define __MGI__

class CMGI
{
public:
    CMGI (CMMSJOS *mMMSJOS, CLcdVdg *mLcdVdg);
    ~CMGI (void);

	boolean Initialize (void);

private:
	CLcdVdg *p_mLcdVdg;
	CMMSJOS *p_mMMSJOS;
	
	static CMGI *s_pThis;
	
	unsigned int  viconef; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
	unsigned int  verro;
    unsigned int vbytepic = 0, vbytevdg;
	unsigned int vcorwb;
	unsigned int vcorwf;
	unsigned char paramVDG[255];
	unsigned char  next_pos;
	unsigned int vxmaxold;
	unsigned int vymaxold; 
	unsigned int voverx; //  Overlay video texto para janelas
	unsigned int vovery; //  Overlay video texto para janelas
	unsigned char vparamstr[255]; //  255 Char Param string
	unsigned int vparam[29]; //  29 Parameters
	unsigned char vbbutton;
	unsigned char vkeyopen;
	unsigned char vbytetec;
	unsigned int pposx;
	unsigned int pposy;
	unsigned int vbuttonwiny;
	unsigned char vbuttonwin[100];
	unsigned int vpostx;
	unsigned int vposty;
	unsigned int  vxgmax;
	unsigned int  vygmax;

	// Funcoes Interface Grafica
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
	void PutIcone(unsigned char* vimage, unsigned int x, unsigned int y);
	void startMGI(void);
	void redrawMain(void);
	void desenhaMenu(void);
	void desenhaIconesUsuario(void);
	void MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone);
	unsigned char editortela(void);
	unsigned char new_menu(void);
	void new_icon(void);
	void del_icon(void);
	void mgi_setup(void);
	void executeCmd(void);
	void InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	void SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
	unsigned char message(char* bstr, unsigned char bbutton, unsigned int btime);
	void showWindow(void);
	void drawButtons(unsigned int xib, unsigned int yib);
	unsigned char waitButton(void);
	void fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo);
	void radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
	void togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
	void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo);
	void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo);
	void VerifyTouchLcd(unsigned char vtipo);
	void SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);
	void RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);
};

#define DrawVertLine(x1, y1, length, color) FillRect(x1, y1, 0, length, color)
#define DrawHoriLine(x1, y1, length, color) FillRect(x1, y1, length, 0, color)

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

#define ICONSPERLINE   8  // Quantidade de Icones por linha
#define SPACEICONS     4  // Quantidade de Espaços entre os Icones Horizontal
#define COLINIICONS   40  // Linha Inicial dos Icones
#define LINHAMENU      36
#define COLMENU       48
#define LINMENU       4

#define ICON_HOME  50
#define ICON_RUN  51
#define ICON_NEW  52
#define ICON_DEL  53
#define ICON_MMSJDOS  54
#define ICON_SETUP  55
#define ICON_EXIT  56
#define ICON_HOURGLASS  57

#endif