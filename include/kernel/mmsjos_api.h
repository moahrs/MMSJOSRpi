/*------------------------------------------------------------------------------
* MMSJ300API.H - Arquivo de Header do MMSJ300
* Author: Moacir Silveira Junior (moacir.silveira@gmail.com)
* Date: 04/07/2013
*------------------------------------------------------------------------------*/

// Alternate definitions
typedef void                    VOID;
typedef char                    CHAR;

typedef struct
{
    unsigned long       firsts;         // Logical block address of the first sector of the FAT partition on the device
    unsigned long       fat;            // Logical block address of the FAT
    unsigned long       root;           // Logical block address of the root directory
    unsigned long       data;           // Logical block address of the data section of the device.
    unsigned int        maxroot;        // The maximum number of entries in the root directory.
    unsigned long       maxcls;         // The maximum number of clusters in the partition.
    unsigned long       sectorSize;     // The size of a sector in bytes
    unsigned long       fatsize;        // The number of sectors in the FAT
    unsigned char        reserv;         // The number of copies of the FAT in the partition
    unsigned char        SecPerClus;     // The number of sectors per cluster in the data region
    unsigned char        type;           // The file system type of the partition (FAT12, FAT16 or FAT32)
    unsigned char        mount;          // Device mount flag (TRUE if disk was mounted successfully, FALSE otherwise)
} DISK;

typedef struct
{
    unsigned char        Name[9];
    unsigned char        Ext[4];
	unsigned char		Attr;
	unsigned int 		CreateDate;
	unsigned int 		CreateTime;
	unsigned int 		LastAccessDate;
	unsigned int 		UpdateDate;
	unsigned int 		UpdateTime;
	unsigned long 		FirstCluster;
	unsigned long       Size;
	unsigned long       DirClusSec;	// Sector in Cluster of the directory (Position calculated)
	unsigned int		DirEntry;	// Entry in directory (step 32)
	unsigned char        Updated;
} FAT32_DIR;

// Pointer to Variables
extern unsigned char arrgDataBuffer[520];
extern unsigned char arrvdiratu[128];
extern unsigned char arrvdiratup[128];
extern unsigned char arrvbufkptr[128];
extern unsigned char arrvbuf[64];
extern unsigned char arrmcfgfile[12288];

extern unsigned char paramVDG[255];
extern unsigned int vxmaxold;
extern unsigned int vymaxold; 
extern unsigned int voverx; //  Overlay video texto para janelas
extern unsigned int vovery; //  Overlay video texto para janelas
extern unsigned char vparamstr[255]; //  255 Char Param string
extern unsigned int vparam[29]; //  29 Parameters
extern unsigned char vbbutton;
extern unsigned char vkeyopen;
extern unsigned char vbytetec;
extern unsigned int pposx;
extern unsigned int pposy;
extern unsigned int vbuttonwiny;
extern unsigned char vbuttonwin[100];
extern unsigned int vpostx;
extern unsigned int vposty;
extern unsigned char  next_pos;

extern unsigned char diskDrv;
extern unsigned long vclusterdir;
extern unsigned long vclusteros;
extern unsigned char  *gDataBuffer; // The global data sector buffer
extern unsigned char  *mcfgfile; // onde eh carregado o arquivo de configuracao e outros arquivos 12K

extern unsigned int  viconef; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
extern unsigned int  vcorf; // cor padrao de frente
extern unsigned int  vcorb; // cor padrao de fundo
extern unsigned int  vcol;
extern unsigned int  vlin;
extern unsigned int  voutput; // 0 - LCD (16x2), 1 - LCD Grafico (320 x 240), 2 - VGA (somente modo texto)
extern unsigned char  *vbuf; // Buffer Linha Digitavel OS 32 bytes
extern unsigned int  vxmax;
extern unsigned int  vymax;
extern unsigned int  xpos;
extern unsigned int  ypos;
extern unsigned int  verro;
extern unsigned char  *vdiratu; // Buffer de pasta atual 128 bytes
extern unsigned char  *vdiratup; // Pointer Buffer de pasta atual 128 bytes (SO FUNCIONA NA RAM)
extern unsigned char  vinip; // Inicio da digitacao do prompt
extern unsigned long vbufk; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
extern unsigned char  *vbufkptr; // Ponteiro para o buffer do teclado (SO FUNCIONA NA RAM)
extern unsigned char  *vbufkmove;
extern unsigned char  *vbufkatu;
extern unsigned char *vbufkbios; // Interface ponteiros entre BIOS e OS para o teclado
extern unsigned char  inten; // Usar interrupcoes para comunicacao com o PIC
extern unsigned int  vxgmax;
extern unsigned int  vygmax;
extern unsigned long vmtaskatu; // Posicao atual no vmtask do prog sendo executado atualmente:
extern unsigned char  vmtask; // Lista de 8 posicoes com 6 Bytes cada posicao:
                                //   - Byte 0 a 1 - ordem de execução dos programas (2 Bytes)
                                //       - 0xAA: programa sendo usado atualmente, 10mS de tempo de execucao
                                //       - 0xFF: programas que estao em 2o.Plano, 1mS de tempo de execucao
                                //   - Bytes 2 a 5 - ordem dos SP's (A7) dos programas (4 Bytes)
extern unsigned char  vmtaskup; // Lista de 8 posicoes com 6 Bytes cada posicao (TOPO)
extern unsigned long intpos; // Posicao da rotina de controle de multitask - OS
extern unsigned long  vtotmem; // Quantidade de memoria total, vindo da bios
extern unsigned int  v10ms; // contador 10mS para programas principais na multitarefa

// Constantes
#define picSendMsg      0x00D0
#define picCarregaSO    0x00D1
#define picReadKbd      0x00D2
#define picDOScmd       0x00D3
#define picloadFile     0x00D4
#define picSectorRead   0x00D7
#define picSectorWrite  0x00D8

#define picIniIntr      0x00EA
#define picIniStatus    0x00EF

#define picErrorNoSO    0x00FD
#define picErrorIO      0x00FE
#define picOkSO         0x00FF

#define picCommData     0x0030
#define picCommStop     0x0040

#define picDOSdir       0x00A0
#define picDOSdel       0x00A1
#define picDOSren       0x00A2
#define picDOSmd        0x00A3
#define picDOScd        0x00A4
#define picDOSrd        0x00A5
#define picDOSformat    0x00A6
#define picDOSdate      0x00A7
#define picDOStime      0x00A8
#define picDOSifconfig  0x00A9
#define picDOScopy      0x00AA

#define NUMBER_OF_DIGITS 32   /* space for NUMBER_OF_DIGITS + '\0' */

#define NOREPOS_CURSOR             0
#define REPOS_CURSOR               1
#define REPOS_CURSOR_ON_CHANGE     2
#define ADD_POS_SCR                3
#define NOADD_POS_SCR              4

//unsigned char  gDataBuffer[512];    // The global data sector buffer

//--- Constantes de DOS
// Estrutura para Nome do Arquivo
typedef struct
{
    unsigned char        Name[9];
    unsigned char        Ext[4];
} FILE_NAME;

// Codigo de Erros
#define ERRO_D_START	      0xFFFFFFF0
#define ERRO_D_FILE_NOT_FOUND 0xFFFFFFF0
#define ERRO_D_READ_DISK      0xFFFFFFF1
#define ERRO_D_WRITE_DISK     0xFFFFFFF2
#define ERRO_D_OPEN_DISK      0xFFFFFFF3
#define ERRO_D_DISK_FULL      0xFFFFFFF4
#define ERRO_D_INVALID_NAME   0xFFFFFFF5
#define ERRO_D_NOT_FOUND      0xFFFFFFFF

#define ERRO_B_START          0xE0
#define ERRO_B_FILE_NOT_FOUND 0xE0
#define ERRO_B_READ_DISK      0xE1
#define ERRO_B_WRITE_DISK     0xE2
#define ERRO_B_OPEN_DISK      0xE3
#define ERRO_B_DIR_NOT_FOUND  0xE5
#define ERRO_B_CREATE_FILE    0xE6
#define ERRO_B_APAGAR_ARQUIVO 0xE7
#define ERRO_B_FILE_FOUND     0xE8
#define ERRO_B_UPDATE_DIR     0xE9
#define ERRO_B_OFFSET_READ    0xEA
#define ERRO_B_DISK_FULL      0xEB
#define ERRO_B_READ_FILE      0xEC
#define ERRO_B_WRITE_FILE     0xED
#define ERRO_B_DIR_FOUND      0xEE
#define ERRO_B_CREATE_DIR     0xEF
#define ERRO_B_NOT_FOUND      0xFF

#define RETURN_OK		      0x00

#define lcdBusy 0x80

#define xlen_lcdg_por 30
#define ylen_lcdg_por 32

#define xlen_lcdg_lan 40
#define ylen_lcdg_lan 24

#define xlen_vga 30
#define ylen_vga 30

#define addline 0x01
#define noaddline 0x00

#define CONV_DATA    0x01
#define CONV_HORA    0x02


//-----------------------------------------------------------------------------
extern unsigned char * _strcat (unsigned char * dst, char * cp, char * src);
extern unsigned int bcd2dec(unsigned int bcd);
extern unsigned int datetimetodir(unsigned char hr_day, unsigned char min_month, unsigned char sec_year, unsigned char vtype);
extern void writechar(int c);
