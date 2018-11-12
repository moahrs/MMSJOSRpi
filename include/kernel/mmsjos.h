/*------------------------------------------------------------------------------
* MMSJ_OS.H - Arquivo de Header do Sistema Operacional
* Author: Moacir Silveira Junior (moacir.silveira@gmail.com)
* Date: 04/07/2013
*------------------------------------------------------------------------------*/

#define FAT32       3

#define TRUE        1
#define FALSE       0

#if !defined(NULL)
    #define NULL        '\0'
#endif

#define MEDIA_SECTOR_SIZE   512

extern unsigned int vcorwb;
extern unsigned int vcorwf;
extern unsigned char *vFinalOS; // Atualizar sempre que a compilacao passar desse valor

#define MEM_POS_MGICFG 16    // 1024 Bytes
                             // mgi_flags = 16 Bytes de flags/config do MGI
                             // icon_pos  = 32 Bytes pos icones
                             // icon_ico  = 320 Bytes (10 por icone (32 icones)) para o nome do arquivo do icone
                             // icon_prg  = 320 Bytes (10 por icone (32 icones)) para o nome do programa do icone

#define MEM_POS_ICONES 1040  // 24 x 24 = 576 Words/Icone = 1152 Bytes/Icone

extern unsigned char strValidChars[];

extern unsigned char vmesc[12][3];

// Demarca o Final do OS, constantes desse tipo nesse compilador vao pro final do codigo.
// Sempre verificar se esta no final mesmo

#define ATTR_READ_ONLY      0x01
#define ATTR_HIDDEN         0x02
#define ATTR_SYSTEM         0x04
#define ATTR_VOLUME         0x08
#define ATTR_LONG_NAME      0x0f
#define ATTR_DIRECTORY      0x10
#define ATTR_DIR_SYSTEM     0x16
#define ATTR_ARCHIVE        0x20
#define ATTR_MASK           0x3f

#define CLUSTER_EMPTY               0x0000
#define LAST_CLUSTER_FAT32      0x0FFFFFFF
#define END_CLUSTER_FAT32       0x0FFFFFF7
#define CLUSTER_FAIL_FAT32      0x0FFFFFFF

#define NUMBER_OF_BYTES_IN_DIR_ENTRY    32
#define DIR_DEL             0xE5
#define DIR_EMPTY           0
#define DIR_NAMESIZE        8
#define DIR_EXTENSION       3
#define DIR_NAMECOMP        (DIR_NAMESIZE+DIR_EXTENSION)

#define EOF             ((int)-1)

#define OPER_READ      0x01
#define OPER_WRITE     0x02
#define OPER_READWRITE 0x03

#define INFO_SIZE    0x01
#define INFO_CREATE  0x02
#define INFO_UPDATE  0x03
#define INFO_LAST    0x04

// Tipos para Cricao/Procura de Arquivos
#define TYPE_DIRECTORY   0x01
#define TYPE_FILE 		 0x02
#define TYPE_EMPTY_ENTRY 0x03
#define TYPE_CREATE_FILE 0x04
#define TYPE_CREATE_DIR  0x05
#define TYPE_DEL_FILE    0x06
#define TYPE_DEL_DIR     0x07
#define TYPE_FIRST_ENTRY 0x08
#define TYPE_NEXT_ENTRY  0x09
#define TYPE_ALL         0xFF

// Tipos para Procura de Clusters
#define FREE_FREE 0x01
#define FREE_USE  0x02
#define NEXT_FREE 0x03
#define NEXT_FULL 0x04
#define NEXT_FIND 0x05


//--- LCD Functions
#define clearLcd() do { \
    *vlcds = 0x01; \
    while ((*vlcds & 0x80) == lcdBusy); \
    *vcol = 0; \
    *vlin = 0; \
} \
while (0)

#define writeCmdLcd(vbyte) do { \
    *vlcds = vbyte; \
    while ((*vlcds & 0x80) == lcdBusy); \
} \
while (0)

#define writeDataLcd(vbyte) do { \
    unsigned int ix; \
    *vlcdd = vbyte; \
    for(ix = 0; ix <= 30; ix++); \
} \
while (0)

//--- Communications Functions
void writestringPic(unsigned char *msg);

//--- Video VGA Functions
#define ativaCursor() do { \
    paramVDG[0] = 0x02; \
    paramVDG[1] = 0xD8; \
    paramVDG[2] = 1; \
    commVDG(paramVDG); \
} \
while (0)

#define blinkCursor() do { \
    paramVDG[0] = 0x01; \
    paramVDG[1] = 0xF0; \
    commVDG(paramVDG); \
} \
while (0)


#define verifKey() do { \
    paramVDG[0] = 0x01; \
    paramVDG[1] = 0xED; \
    commVDG(paramVDG); \
} \
while (0)

#define getKey() do { \
    paramVDG[0] = 0x01; \
    paramVDG[1] = 0xDC; \
    commVDG(paramVDG); \
} \
while (0)

//--- OS Functions
extern void processCmd(void);
extern void clearScr(unsigned int pcolor);
extern void writes(char *msgs, unsigned int pcolor, unsigned int pbcolor);
extern void writec(unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor, unsigned char ptipo);
extern void putPrompt(unsigned int plinadd);
extern void locate(unsigned char pcol, unsigned char plin, unsigned char pcur);
extern unsigned long loadFile(unsigned char *parquivo, unsigned short* xaddress);
extern void runCmd(void);
extern unsigned char loadCFG(unsigned char ptipo);
extern void catFile(unsigned char *parquivo);
extern void funcKey(unsigned char vambiente, unsigned char vshow, unsigned char venter, unsigned char vtipo, unsigned int x, unsigned int y);

// Funcoes Interface Grafica
extern void locatexy(unsigned int pposx, unsigned int ppoxy);
extern void writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor);
extern void writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor);
extern void SetDot(unsigned int x, unsigned int y, unsigned int color);
extern void FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor);
extern void DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color);
extern void DrawRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight, unsigned int color);
extern void DrawRoundRect(void);
extern void DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor);
extern void PutImage(unsigned char* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
extern void PutIcone(unsigned char* vimage, unsigned int x, unsigned int y);
extern void startMGI(void);
extern void redrawMain(void);
extern void desenhaMenu(void);
extern void desenhaIconesUsuario(void);
extern void MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone);
extern unsigned char editortela(void);
extern unsigned char new_menu(void);
extern void new_icon(void);
extern void del_icon(void);
extern void mgi_setup(void);
extern void executeCmd(void);
extern void verifyMGI(void);
extern void InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
extern void SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight);
extern unsigned char message(char* bstr, unsigned char bbutton, unsigned int btime);
extern void showWindow(void);
extern void drawButtons(unsigned int xib, unsigned int yib);
extern unsigned char waitButton(void);
extern void fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo);
extern void radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
extern void togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo);
extern void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo);
extern void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo);
extern void VerifyTouchLcd(unsigned char vtipo);
extern void SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);
extern void RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage);

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

//--- FAT32 Functions
extern unsigned char fsFormat (long int serialNumber, char * volumeID);
extern unsigned char fsMountDisk(void);
extern void fsSetClusterDir (unsigned long vclusdiratu);
extern unsigned long fsGetClusterDir (void);
extern unsigned char fsSectorWrite(unsigned long vsector, unsigned char* vbuffer, unsigned char vtipo);
extern unsigned char fsSectorRead(unsigned long vsector, unsigned char* vbuffer);

// Funcoes de Manipulacao de Arquivos
extern unsigned char fsCreateFile(char * vfilename);
extern unsigned char fsOpenFile(char * vfilename);
extern unsigned char fsCloseFile(char * vfilename, unsigned char vupdated);
extern unsigned long fsInfoFile(char * vfilename, unsigned char vtype);
extern unsigned char fsRWFile(unsigned long vclusterini, unsigned long voffset, unsigned char *buffer, unsigned char vtype);
extern unsigned char fsReadFile(char * vfilename, unsigned long voffset, unsigned char *buffer, unsigned char vsizebuffer);
extern unsigned char fsWriteFile(char * vfilename, unsigned long voffset, unsigned char *buffer, unsigned char vsizebuffer);
extern unsigned char fsDelFile(char * vfilename);
extern unsigned char fsRenameFile(char * vfilename, char * vnewname);

// Funcoes de Manipulacao de Diretorios
extern unsigned char fsMakeDir(char * vdirname);
extern unsigned char fsChangeDir(char * vdirname);
extern unsigned char fsRemoveDir(char * vdirname);
extern unsigned char fsPwdDir(unsigned char *vdirpath);

// Funcoes de Apoio
extern unsigned long fsFindInDir(char * vname, unsigned char vtype);
extern unsigned char fsUpdateDir(void);
extern unsigned long fsFindNextCluster(unsigned long vclusteratual, unsigned char vtype);
extern unsigned long fsFindClusterFree(unsigned char vtype);
extern void runCmd(void);
