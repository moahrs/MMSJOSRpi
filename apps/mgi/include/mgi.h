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
	

	// Funcoes Interface Grafica
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