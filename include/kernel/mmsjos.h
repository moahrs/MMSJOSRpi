/*------------------------------------------------------------------------------
* MMSJOS.H - Arquivo de Header do Sistema Operacional
* Author: Moacir Silveira Junior (moacir.silveira@gmail.com)
* Date: 02/11/2018
*------------------------------------------------------------------------------*/

#ifndef __MMSJOS__
#define __MMSJOS__

#define FAT32       3

#if !defined(TRUE)
    #define TRUE        1
#endif

#if !defined(FALSE)
    #define FALSE       0
#endif

#if !defined(NULL)
    #define NULL        '\0'
#endif

typedef struct tagMMSJConfig
{
	unsigned int   TextColorF;
	unsigned int   TextColorB;
	unsigned int   MgiColorF;
	unsigned int   MgiColorB;
	unsigned int   StartEnv;
	unsigned int   TypeComm;
} MMSJCONFIG;


class CMMSJOS
{
public:
    CMMSJOS (CInterruptSystem *mInterrupt, CTimer *mTimer, CDWHCIDevice *mDWHCI, FATFS *mFileSystem, CEMMCDevice *mEMMC);
    ~CMMSJOS (void);

	boolean Initialize (void);

	void Start(void);

	unsigned char  *vbuf; // Buffer Linha Digitavel OS 32 bytes

	unsigned char  *mcfgfile; // onde eh carregado o arquivo de configuracao e outros arquivos 12K

	static CMMSJOS *Get (void);
	void processCmd(void);
	void load232(void);
	unsigned long loadFile(unsigned char *parquivo, unsigned char* xaddress);
	unsigned char* vMemSystemArea = (unsigned char*)0x00700000;       // 1MB - Atualizar sempre que a compilacao passar desse valor
	unsigned char* vMemUserArea = (unsigned char*)0x00800000;   // 440MB - Atualizar sempre que a compilacao passar desse valor
	char * _strcat (char *pDest, const char *pSrc);
	unsigned int bcd2dec(unsigned int bcd);
	unsigned int datetimetodir(unsigned char hr_day, unsigned char min_month, unsigned char sec_year, unsigned char vtype);
	unsigned char vmesc[12][3] = {{'J','a','n'},{'F','e','b'},{'M','a','r'},
	                           {'A','p','r'},{'M','a','y'},{'J','u','n'},
	                           {'J','u','l'},{'A','u','g'},{'S','e','p'},
	                           {'O','c','t'},{'N','o','v'},{'D','e','c'}};

private:
	CBTSubSystem *p_mBluetooth;
	CInterruptSystem * p_mInterrupt;
	CTimer *p_mTimer;
	CDWHCIDevice *p_mDWHCI;
	FATFS *p_mFileSystem;
	CEMMCDevice *p_mEMMC;
    CKeyboardBuffer *p_mbufKeyboard;

	MMSJCONFIG vMMSJConfig;

	#ifdef __USE_TFT_LCD__
		CScrTft *p_mOut;
	#else
		CScreenDevice *p_mOut;
	#endif
	CUSBKeyboardDevice *pKeyboard;

	static char statusUartInit;
	static CMMSJOS *s_pThis;
	static char vkeybuffer[255];

	unsigned char* pVersionSO = (unsigned char*)"0.7";

    unsigned int vbytepic = 0, vbytevdg;
    unsigned char *vbufptr;

	unsigned long vtotclock;
	unsigned long vArmCpuId;
	unsigned int vboardfirm;
	unsigned int vboardmodel;
	unsigned int vboardrev;
	unsigned char vboardmacaddr[8];
	unsigned long long vboardserial;

	unsigned char paramVDG[255];

	unsigned char *vFinalOS; // Atualizar sempre que a compilacao passar desse valor
	unsigned char arrgDataBuffer[520];
	unsigned char arrvdiratu[128];
	unsigned char arrvbufkptr[128];
	unsigned char arrvbuf[64];
	unsigned char arrmcfgfile[180000];

	unsigned char vparamstr[255]; //  255 Char Param string
	unsigned int vparam[29]; //  29 Parameters
	unsigned char vbbutton;
	unsigned char vkeyopen;
	unsigned char vbytetec;
	unsigned int vbuttonwiny;
	unsigned char vbuttonwin[100];

	unsigned char diskDrv;
	unsigned long vclusterdir;
	unsigned long vclusteros;
	unsigned char  *gDataBuffer; // The global data sector buffer

	unsigned int  verro;
	unsigned char  vdiratu[128]; // Buffer de pasta atual 128 bytes
	unsigned char  *vdiratup; // Pointer Buffer de pasta atual 128 bytes (SO FUNCIONA NA RAM)
	unsigned char  vinip; // Inicio da digitacao do prompt
	unsigned long vbufk; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
	unsigned char  *vbufkptr; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
	unsigned char  *vbufkmove;
	unsigned char  *vbufkatu;
	unsigned char *vbufkbios; // Interface ponteiros entre BIOS e OS para o teclado
	unsigned char  inten; // Usar interrupcoes para comunicacao com o PIC
	unsigned int  vxgmax;
	unsigned int  vygmax;
	unsigned long vmtaskatu; // Posicao atual no vmtask do prog sendo executado atualmente:
	unsigned char  vmtask; // Lista de 8 posicoes com 6 Bytes cada posicao:
	                                //   - Byte 0 a 1 - ordem de execução dos programas (2 Bytes)
	                                //       - 0xAA: programa sendo usado atualmente, 10mS de tempo de execucao
	                                //       - 0xFF: programas que estao em 2o.Plano, 1mS de tempo de execucao
	                                //   - Bytes 2 a 5 - ordem dos SP's (A7) dos programas (4 Bytes)
	unsigned char  vmtaskup; // Lista de 8 posicoes com 6 Bytes cada posicao (TOPO)
	unsigned long intpos; // Posicao da rotina de controle de multitask - OS
	unsigned long  vtotmem; // Quantidade de memoria total, vindo da bios
	unsigned int  v10ms; // contador 10mS para programas principais na multitarefa

	unsigned int vxmaxold;
	unsigned int vymaxold; 
	unsigned int voverx; //  Overlay video texto para janelas
	unsigned int vovery; //  Overlay video texto para janelas
	unsigned char  next_pos;
	unsigned int vpostx;
	unsigned int vposty;

	unsigned char vmgiflags[255];

	unsigned char vTimerShowed;

	TCHAR * retPathAndFile(char * cFileName);
	int fsMount(void);
	void putPrompt(unsigned int plinadd);
	void runCmd(void);
	unsigned char loadCFG(void);
	void catFile(unsigned char *parquivo);
	static void KeyPressedHandler (const char *pString);
	static void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]);

	static void TimerHandler (unsigned hTimer, void *pParam, void *pContext);
	
	void startMGI(void);
	void PutIcone(unsigned char* vimage, unsigned int x, unsigned int y);
	void redrawMain(void);
	void redrawMainRest(void);
	unsigned char editortela(void);
	void desenhaMenu(void);
	void desenhaTimer(void);
	void desenhaIconesUsuario(void);
	void MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone);
	unsigned char showMenu(void);
	void executeCmd(void);
	void new_icon(void);
	void del_icon(void);
	unsigned char message(char* bstr, unsigned char bbutton, unsigned int btime);
	void setupMGI(void);
    void setupDateTimer(void);

	void loadImage(unsigned int px, unsigned int py, unsigned int pw, unsigned int ph, char *filename, char pType);
};


#define MEM_POS_MGICFG 16    // 1024 Bytes
                             // mgi_flags = 16 Bytes de flags/config do MGI
                             // icon_pos  = 32 Bytes pos icones
                             // icon_ico  = 320 Bytes (10 por icone (32 icones)) para o nome do arquivo do icone
                             // icon_prg  = 320 Bytes (10 por icone (32 icones)) para o nome do programa do icone

#define MEM_POS_ICONES 1040  // 24 x 24 = 576 Words/Icone = 1152 Bytes/Icone

#ifndef EOF
#define EOF             ((int)-1)
#endif

//--- OS Functions

// Alternate definitions
typedef void                    VOID;
typedef char                    CHAR;

// Constantes
#define NUMBER_OF_DIGITS 32   /* space for NUMBER_OF_DIGITS + '\0' */

//unsigned char  gDataBuffer[512];    // The global data sector buffer

// Codigo de Erros
#define RETURN_OK		      0x00

#define CONV_DATA    0x01
#define CONV_HORA    0x02

// Graphical Functions
#define ICONSPERLINE   8  // Quantidade de Icones por linha
#define SPACEICONS     4  // Quantidade de Espaços entre os Icones Horizontal
#define COLINIICONS   40  // Linha Inicial dos Icones
#define LINHAMENU      36
#define QTDITENSMENU   6
#define QTDCOLSMENU    3
#define QTDLINSMENU   2

#define COLMENU       1 // 48
#define LINMENU       1 // 4

#define ICON_HOME  50
#define ICON_RUN  51
#define ICON_NEW  52
#define ICON_DEL  53
#define ICON_MMSJDOS  54
#define ICON_SETUP  55
#define ICON_EXIT  56
#define ICON_HOURGLASS  57
#define ICON_FILES 58
#define ICON_HELP 59

#define IMGFORMATBMP 0
#define IMGFORMATICO 1

//-----------------------------------------------------------------------------
#endif
