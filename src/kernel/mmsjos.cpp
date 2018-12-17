/********************************************************************************
*    Programa    : mmsjos.cpp
*    Objetivo    : SO do modulo RASPBERRY PI ZERO W
*    Criado em   : 02/11/2018
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versão  Responsavel  Motivo
* 02/11/2018  0.1     Moacir Jr.   Criação Versão Beta
* 12/11/2018  0.2     Moacir Jr.   Adaptar uso USB
* 20/11/2018  0.3     Moacir Jr.   Adaptar uso Bluetooth
* 23/11/2018  0.4     Moacir Jr.   Mudança de Planos, Utilizar Circle lib, by rsta2
*                                  utilizando LCDVDG(TFT) em vez de screen padrão                                  
*                                  SO ficara com comandos e demais controles proprio
*                                  do SO. 
* 01/12/2018  0.5     Moacir Jr.   Desistencia de usar bluetooth e wifi da placa zero w
*                                  por falta de informações por parte do(s) fabricante(s)
*                                  Optou-se por usar modulo HM-10 para bluetooth e o
*                                  modulo esp8266 para wifi. Usar UART do mmsjos, e nao serial 
*                                  do Circle lib, e desenv soft serial (ver pinos)
* 07/12/2018  0.6     Moacir Jr.   SO por si mesmo. Deixará de ter is drivers incorporados.
*                                  drivers, como bluetooth, wifi (modulo ou nativo placa), 
*                                  serao desenvolvidos a parte. No SO somente 
*                                  Video (hdmi ou tft), FileSystem, serial, timer e etc 
*                                  (basico do bcm2835).
* 11/12/2018  0.7     Moacir Jr.   Colocar Grafico  
*--------------------------------------------------------------------------------
* ---------------
* Mapa de Memoria
* ---------------
*
*   SDRAM 512MB
* +-------------+ 00000000h
* | SYSTEM 32KB |
* +-------------+ 00007FFFh
* |             | 00008000h
* | OS & LCDVDG |
* |     8MB     |
* |             |
* +-------------+ 007FFFFFh
* |             | 00800000h
* |             |
* |             |
* |             |
* |             |
* |             |
* |             |
* |  USER AREA  |
* |    440MB    |
* |             |
* |             |
* |             |
* |             |
* |             |
* |             |
* |             |
* |             | 
* +-------------+ 1BFFFFFFh
* |             | 1C000000h
* |             |
* |  GPU 64MB   | 
* |             |
* |             |
* +-------------+ 1FFFFFFFh 
*
********************************************************************************
*
* Icones MGI
* 50 = Home
* 51 = Run
* 52 = New
* 53 = trash
* 54 = MMSJDOS
* 55 = Setup MGI
* 56 = Exit
* 57 = Hourglass
* 58 = Files
* 59 = Help
*
********************************************************************************/

#define __USE_TFT_SCROLL__

#include <circle/usb/usbkeyboard.h>
#include <circle/input/keyboardbuffer.h>
#include <circlelib/circlelib.h>
#include <circle/machineinfo.h>
#include <circle/debug.h>
#include <circle/util.h>
#include <circle/alloc.h>
#include <SDCard/emmc.h>
#include <stddef.h>
#include <stdint.h>
#include <drivers/uart.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#include "common/mylib.h"
#include <drivers/ds1307.h>

#define DRIVE       "SD:"

int __locale_ctype_ptr(int return_value)
{
    (void)(return_value);
    return 0;
}   

char CMMSJOS::statusUartInit = 0;
CMMSJOS *CMMSJOS::s_pThis = 0;
char CMMSJOS::vkeybuffer[255];

const char* pMonthName[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* pIconsMgi[10] = {"mgihome.ico", "mgirun.ico", "mginew.ico", "mgitrash.ico", "mgimdos.ico", "mgisetup.ico", "mgiexit.ico", "mgiglass.ico", "mgifiles.ico", "mgihelp.ico"};
const int pItensMenuHome[QTDITENSMENU] = {ICON_RUN, ICON_MMSJDOS, ICON_FILES, ICON_SETUP, ICON_DEL, ICON_HELP};

CMMSJOS::CMMSJOS (CInterruptSystem *mInterrupt, CTimer *mTimer, CDWHCIDevice *mDWHCI, FATFS *mFileSystem, CEMMCDevice *mEMMC)
: p_mInterrupt (mInterrupt),
  p_mTimer (mTimer),  
  p_mDWHCI (mDWHCI), 
  p_mFileSystem (mFileSystem), 
  p_mEMMC (mEMMC)
{
    s_pThis = this;
    p_mBluetooth = 0;

    #ifdef __USE_TFT_LCD__
        p_mOut = CScrTft::Get();
    #else
        p_mOut = CScreenDevice::Get();
    #endif
}

CMMSJOS::~CMMSJOS (void)
{
    s_pThis = 0;
    p_mTimer = 0;
    p_mBluetooth = 0;
    p_mDWHCI = 0;
    p_mFileSystem = 0;
}

boolean CMMSJOS::Initialize (void)
{
    int pResM;

    CBcmPropertyTags Tags;
    TPropertyTagSerial TagSerial;
    Tags.GetTag (PROPTAG_GET_BOARD_SERIAL, &TagSerial, sizeof TagSerial, 4);

    TPropertyTagMACAddress MACAddress;
    Tags.GetTag (PROPTAG_GET_MAC_ADDRESS, &MACAddress, sizeof MACAddress);

    vtotclock = CMachineInfo::Get ()->GetClockRate (CLOCK_ID_ARM);

    TPropertyTagMemory TagMemory;
    Tags.GetTag (PROPTAG_GET_ARM_MEMORY, &TagMemory, sizeof TagMemory);
    vtotmem = (TagMemory.nSize - (8 << 20) );

    vbufkptr = (unsigned char*)malloc(128);
    vbuf = (unsigned char*)malloc(64);
    mcfgfile = (unsigned char*)malloc(180000);
    vbufptr = vbuf;
    *vbufptr = 0x00;

    vtotmem = (vtotmem >> 20);
    vtotclock = (vtotclock / 1000000);

    vMMSJConfig.TextColorF = Green;
    vMMSJConfig.TextColorB = Black;
    vMMSJConfig.MgiColorF = Black;
    vMMSJConfig.MgiColorB = l_gray;
    vMMSJConfig.StartEnv = 0;
    vMMSJConfig.TypeComm = 0;

    p_mOut->ativaCursor();

    printf("OS> MMSJ-OS version %s\n", pVersionSO);
    printf("OS> %s at %d%sHz\n", CMachineInfo::Get ()->GetMachineName (), ((vtotclock >= 1000) ? (vtotclock / 1000) : vtotclock), ((vtotclock >= 1000) ? "G" : "M"));
    printf("OS> Model %04x\n", CMachineInfo::Get ()->GetMachineModel ());
    printf("OS> Revision %04x\n", CMachineInfo::Get ()->GetModelRevision ());
    printf("OS> Serial %08x%08x\n", TagSerial.Serial[0], TagSerial.Serial[1]);
    printf("OS> MAC %02x.%02x.%02x.%02x.%02x.%02x\n", MACAddress.Address[0], MACAddress.Address[1], MACAddress.Address[2], MACAddress.Address[3], MACAddress.Address[4], MACAddress.Address[5]);
    printf("OS> Total Free Memory %dMB\n", vtotmem);
    printf("OS> LCD Graphic %dx%d\n", (p_mOut->GetWidth() + 1), (p_mOut->GetHeight() + 1));
    printf("OS> LCD Text %dx%d\n", (p_mOut->GetColumns() + 1), (p_mOut->GetRows() + 1));
    printf("OS> Touch Module Found\n");

    if (bcm2835_gpio_lev(41))
        printf("OS> Wifi is On\n");

    if (bcm2835_gpio_lev(45))
        printf("OS> Bluetooth is On\n");

    pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get ()->GetDevice ("ukbd1", FALSE);

    if (pKeyboard != 0)
    {
        printf("OS> USB Keyboard Found\n");
        p_mbufKeyboard = new CKeyboardBuffer(pKeyboard);
    }

    pResM = fsMount();
    if (pResM == FR_OK)
        printf("OS> Mounting Disk... Done\n");
    else
        printf("OS> Mounting Disk... Error (%d)\n",pResM);

    loadCFG();

    return TRUE;
}

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void CMMSJOS::Start(void)
{
    int ikx, nResult = 0;

    // Inicio SO
    strcpy((char*)vdiratu,"/\0");

    if (vMMSJConfig.StartEnv == 1) 
        startMGI();

    printf("\n");
    putPrompt(noaddline);

    p_mOut->ativaCursor();

    // Ativa Teclado Touch
    if (pKeyboard == 0)
        p_mOut->funcKey(0,1, 1, 0, 50, 0);

    // Loop principal
    while (1) {
        // Verificar Teclado Touch
        if (pKeyboard == 0)
        {
            vbytepic = p_mOut->getKey();
        }
        else 
        {
            pKeyboard->UpdateLEDs();
            vbytepic = 0x00;

            if (nResult <= 0) 
            {
                nResult = p_mbufKeyboard->Read (vkeybuffer, sizeof vkeybuffer);

                if (nResult > 0)
                    ikx = 0;
            }

            if (nResult > 0) 
            {
                vbytepic = vkeybuffer[ikx++];
                nResult--;
            }
        }

        // Verifica Retorno
        if (vbytepic != 0) {
            if (vbytepic >= 0x20 && vbytepic <= 0x7E) {
                *vbufptr = vbytepic;
                vbufptr++;

                if (vbufptr > vbuf + 31)
                    vbufptr = vbuf + 31;

                if (p_mOut->GetColumn() > p_mOut->GetColumns()) {
                    p_mOut->SetRow(p_mOut->GetRow() + 1);
                    p_mOut->locate(0, p_mOut->GetRow(), NOREPOS_CURSOR);
                }

                p_mOut->writec(vbytepic, ADD_POS_SCR);
            }
            else {
                switch (vbytepic) {
                    case 0x0A:  // New Line
                    case 0x0D:  // Enter
                        p_mOut->SetRow(p_mOut->GetRow() + 1);
                        p_mOut->desativaCursor();
                        if (pKeyboard == 0)
                            p_mOut->funcKey(0,2, 0, 0, 50, 0);
                        p_mOut->locate(0, p_mOut->GetRow(), NOREPOS_CURSOR);
                        *vbufptr = 0x00;
                        processCmd();
                        putPrompt(noaddline);
                        vbufptr = vbuf;
                        *vbufptr = 0x00;
                        if (pKeyboard == 0)
                            p_mOut->funcKey(0,1, 1, 0, 50, 0);
                        break;
                    case 0x7F:  // USB Keyboard BackSpace
                    case 0x08:  // Touch Keyboard BackSpace
                        if (p_mOut->GetColumn() > vinip) {
                            *vbufptr = '\0';
                            vbufptr--;
                            if (vbufptr < vbuf)
                                vbufptr = vbuf;
                            *vbufptr = '\0';
                            p_mOut->SetColumn(p_mOut->GetColumn() - 1);
                            p_mOut->locate(p_mOut->GetColumn(),p_mOut->GetRow(), NOREPOS_CURSOR);
                            p_mOut->writec(0x08, ADD_POS_SCR);
                            p_mOut->SetColumn(p_mOut->GetColumn() - 1);
                            p_mOut->locate(p_mOut->GetColumn(),p_mOut->GetRow(), NOREPOS_CURSOR);
                        }
                        break;
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Comm Functions
//-----------------------------------------------------------------------------
CMMSJOS *CMMSJOS::Get (void)
{
    assert (s_pThis != 0);
    return s_pThis;
}

//-----------------------------------------------------------------------------
// OS Functions
//-----------------------------------------------------------------------------
void CMMSJOS::processCmd(void) 
{
    char linhacomando[32], linhaarg[32];
    unsigned char *blin = (unsigned char*)vbuf;
    char vlinha[40], vproccd[32];
    unsigned int varg = 0, pxx, pyy;
    unsigned int ix, iy, iz = 0, ikk = 0;
    unsigned int vbytepic = 0, ikx, iky, ikz;
    unsigned char vparam[32], vparam2[16], vpicret;
    unsigned long vretfat, pRetWriteData;
    time_t pData;
    FRESULT pResF;

    // Separar linha entre comando e argumento
    linhacomando[0] = '\0';
    linhaarg[0] = '\0';
    ix = 0;
    iy = 0;
    while (*blin != 0) 
    {
        if (!varg && *blin == 0x20) 
        {
            varg = 0x01;
            linhacomando[ix] = '\0';
            iy = ix;
            ix = 0;
        }
        else 
        {
            if (!varg)
                linhacomando[ix] = toupper(*blin);
            else
                linhaarg[ix] = *blin;
            ix++;
        }

        blin++;
    }

    if (!varg) 
    {
        linhacomando[ix] = '\0';
        iy = ix;
    }
    else 
    {
        linhaarg[ix] = '\0';

        ikk = 0;
        iz = 0;
        varg = 0;
        while (ikk < ix) {
            if (linhaarg[ikk] == 0x20)
                varg = 1;
            else 
            {
                if (!varg)
                    vparam[ikk] = linhaarg[ikk];
                else {
                    vparam2[iz] = linhaarg[ikk];
                    iz++;
                }
            }

            ikk++;
        }
    }

    vparam[ikk] = '\0';
    vparam2[iz] = '\0';

    vpicret = 0;

    // Processar e definir o que fazer
    if (linhacomando[0] != 0) 
    {
        if (!strcmp(linhacomando,"MMSJDOS") && iy == 7) 
        {
            // ver como fazer
        }
        else if (!strcmp(linhacomando,"CLS") && iy == 3) 
        {
            p_mOut->clearScr();
        }
        else if (!strcmp(linhacomando,"CLEAR") && iy == 5) 
        {
            p_mOut->clearScr();
        }
        else if (!strcmp(linhacomando,"VER") && iy == 3) 
        {
            printf("MMSJ-OS v%s\n", pVersionSO);
        }
        else if (!strcmp(linhacomando,"MGI") && iy == 3) 
        {
            startMGI();
        }
        else if (!strcmp(linhacomando,"VIMG") && iy == 4) 
        {
            loadImage(0,0,320,240,(char*)vparam,IMGFORMATBMP);
            p_mOut->VerifyTouchLcd(WHAITTOUCH, &pxx, &pyy);
            p_mOut->clearScr();            
        }
        else if (!strcmp(linhacomando,"LS") && iy == 2) 
        {
            int dirDay, dirMonth /*, dirYear*/ ;
            int dirHour, dirMin;
            DIR Directory;
            FILINFO FileInfo;
            FRESULT Result = f_findfirst (&Directory, &FileInfo, (char*)vdiratu, "*");
            printf("\n");

            #if FF_USE_LABEL
                TCHAR* labelDisk;
                DWORD* serieDisk;
                if (f_getlabel (DRIVE, labelDisk, serieDisk) == FR_OK)
                {
                    CString DiskLabel;
                    DiskLabel.Format ("%-17s", labelDisk);
                    printf("       Name Disk is %s\n", DiskLabel);
                    printf("       Serial Disk is %04x-%04x\n", ((*serieDisk & 0xFFFF0000) >> 16), (*serieDisk & 0x0000FFFF));
                }
            #endif

            for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
            {
                if (!(FileInfo.fattrib & (AM_HID | AM_SYS )))
                {
                    if (FileInfo.fattrib == AM_DIR)
                        strcat(FileInfo.fname,"/\0");

                    CString FileName;
                    FileName.Format ("%-17s", FileInfo.fname);
                    FileInfo.fname[17] = 0x00;
                    dirDay   = FileInfo.fdate & 0x001F;
                    dirMonth = ((FileInfo.fdate & 0x01E0) >> 5) - 1;
//                    dirYear  = ((FileInfo.fdate & 0xFE00) >> 9) + 1980;

                    dirHour = ((FileInfo.fdate & 0xF800) >> 11);
                    dirMin  = ((FileInfo.fdate & 0x07E0) >> 5);

                    printf("%8d %s %02d %02d:%02d %s\n", FileInfo.fsize, pMonthName[dirMonth], dirDay, dirHour, dirMin, (const char *) FileName);
                }

                Result = f_findnext (&Directory, &FileInfo);
            }
            printf("\n");
        }
        else 
        {
            if (!strcmp(linhacomando,"RM") && iy == 2) 
            {
                pResF = f_unlink(retPathAndFile((char*)vparam));
                if (pResF != FR_OK)
                {
                    printf("Delete File/Folder Error (%d)\n", pResF);
                }
                else
                    printf("\n");
            }
            else if (!strcmp(linhacomando,"REN") && iy == 3) 
            {
                pResF = f_rename(retPathAndFile((char*)vparam), (char*)vparam2);

                if (pResF != FR_OK)
                {
                    printf("Rename Error (%d)\n", pResF);
                }
                else
                    printf("\n");
            }
            else if (!strcmp(linhacomando,"CP") && iy == 2) 
            {
                /* ver como pegar */
            }
            else if (!strcmp(linhacomando,"MD") && iy == 2) 
            {
                pResF = f_mkdir(retPathAndFile((char*)vparam));
                if (pResF != FR_OK)
                {
                    printf("Create Folder Error (%d)\n", pResF);
                }
                else
                    printf("\n");
            }
            else if (!strcmp(linhacomando,"CD") && iy == 2) 
            {            
                if (!strcmp((char*)vparam,".") && strlen((char*)vparam) == 1)
                    printf("%s\n",vdiratu);
                else
                {
                    iky = strlen((char*)vparam) + 1;
                    ikz = 0;
                    vproccd[0] = 0x00;
                    for(ikx = 0; ikx < iky; ikx++)
                    {
                        if (vparam[ikx] != '/' && vparam[ikx] != 0x00)
                            vproccd[ikz++] = vparam[ikx];
                        else
                        {
                            vproccd[ikz] = 0x00;
                            ikz = 0;

                            if (!strcmp((char*)vproccd,".."))
                            {
                                iy = strlen((char*)vdiratu) + 1;
                                for(ix = iy; ix >= 1; ix--)
                                {
                                    if (vdiratu[ix] == '/')
                                    {
                                        vdiratu[ix] = 0x00;
                                        break;
                                    }

                                    vdiratu[ix] = 0x00;
                                }
                            }
                            else
                            {
                                DIR Directory;
                                FILINFO FileInfo;
                                pResF = f_findfirst (&Directory, &FileInfo, (char*)vdiratu, (char*)vproccd);
                                if (pResF != FR_OK || (pResF == FR_OK && FileInfo.fattrib != AM_DIR))
                                {
                                    printf("Path Not Found (%d)\n", pResF);
                                    break;
                                }
                                else 
                                {
                                    if (!strcmp(FileInfo.fname,(char*)vproccd))
                                    {
                                        if (strlen((char*)vdiratu) > 1)
                                            strcat((char*)vdiratu,"/\0");
                                        strcat((char*)vdiratu,(char*)vparam);
                                    }
                                    else
                                    {
                                        printf("Path Not Found (%d)\n", pResF);
                                        break;
                                    }
                                }
                            }

                            vproccd[0] = 0x00;

                            if (vparam[ikx] == 0x00)
                                break;
                        }

                    }
                }
                printf("\n");
            }
            else if (!strcmp(linhacomando,"TIMER") && iy == 5) 
            {
                unsigned char strDay[3], strMon[3], strYear[3];
                unsigned char strHrs[3], strMin[3], strSec[3], strPtr[3];

                pData = ds1307_read();

                if (vparam[0] || vparam2[0])
                {
                    if (vparam[0])
                    {
                        strPtr[0] = vparam[0];
                        strPtr[1] = vparam[1];
                        strPtr[2] = 0;
                        pData.Day = atoi((char*)strPtr);
                        strPtr[0] = vparam[3];
                        strPtr[1] = vparam[4];
                        strPtr[2] = 0;
                        pData.Month = atoi((char*)strPtr);
                        strPtr[0] = vparam[8];
                        strPtr[1] = vparam[9];
                        strPtr[2] = 0;
                        pData.Year = atoi((char*)strPtr);
                    }

                    if (vparam2[0])
                    {
                        strPtr[0] = vparam2[0];
                        strPtr[1] = vparam2[1];
                        strPtr[2] = 0;
                        pData.Hour = atoi((char*)strPtr);
                        strPtr[0] = vparam2[3];
                        strPtr[1] = vparam2[4];
                        strPtr[2] = 0;
                        pData.Minute = atoi((char*)strPtr);
                        strPtr[0] = vparam2[6];
                        strPtr[1] = vparam2[7];
                        strPtr[2] = 0;
                        pData.Second = atoi((char*)strPtr);
                    }

                    pRetWriteData = ds1307_write(pData);

                    if (pRetWriteData) 
                        printf("  Write RTC Error (%d)\n",pData.Error);

                    pData = ds1307_read();
                }

                if (!pData.Error) 
                {
                    strDay[0] = BCD2UpperCh(pData.Day);
                    strDay[1] = BCD2LowerCh(pData.Day);
                    strDay[2] = 0;
                    strMon[0] = BCD2UpperCh(pData.Month);
                    strMon[1] = BCD2LowerCh(pData.Month);
                    strMon[2] = 0;
                    strYear[0] = BCD2UpperCh(pData.Year);
                    strYear[1] = BCD2LowerCh(pData.Year);
                    strYear[2] = 0;

                    strHrs[0] = BCD2UpperCh(pData.Hour);
                    strHrs[1] = BCD2LowerCh(pData.Hour);
                    strHrs[2] = 0;
                    strMin[0] = BCD2UpperCh(pData.Minute);
                    strMin[1] = BCD2LowerCh(pData.Minute);
                    strMin[2] = 0;
                    strSec[0] = BCD2UpperCh(pData.Second);
                    strSec[1] = BCD2LowerCh(pData.Second);
                    strSec[2] = 0;

                    printf("  Date is (dd/mm/yyyy) %s/%s/20%s\n", strDay, strMon, strYear);
                    printf("  Time is (hh:mm:ss) %s:%s:%s\n", strHrs, strMin, strSec);
                }
                else
                    printf("  Read RTC Error (%d)\n",pData.Error);
            }
            else if (!strcmp(linhacomando,"LSDEV") && iy == 5)
            {
                printf("\n");
                CDeviceNameService::Get ()->ListDevices (p_mOut);
                printf("\n");

            }
            else if (!strcmp(linhacomando,"IFCONFIG") && iy == 8) 
            {
                vpicret = 1;
                /* ver como pegar */
            }
            else if (!strcmp(linhacomando,"LOADCFG") && iy == 7) 
            {
                loadCFG();
                ix = 255;
            }
            else if (!strcmp(linhacomando,"INIT232") && iy == 7)
            {
                // Incializa Serial
                if (p_mBluetooth != 0)
                {
                    delete p_mBluetooth;
                    p_mBluetooth = 0;
                }

                uart_init();

                statusUartInit = 1;
                
                printf("OS> Uart Initialized...\n");
            }
            else if (!strcmp(linhacomando,"LOAD232") && iy == 7) 
            {
                load232();
                ix = 255;
            }
            else if (!strcmp(linhacomando,"SEND232") && iy == 7) 
            {
                // A definir
                ix = 255;
            }
            else if (!strcmp(linhacomando,"SETBAUD") && iy == 7)
            {
                int iBaud = atoi((char*)vparam);
                int rBaud = uart_setbaud(iBaud);

                printf("Serial BaudRate Changed to %dbps\n\n", rBaud);
            }
            else if (!strcmp(linhacomando,"CAT") && iy == 3) 
            {
                catFile((unsigned char*)linhaarg);
                ix = 255;
            }
            else 
            {
                // Verifica se tem Arquivo com esse nome na pasta atual no disco
/*                ix = iy;
                linhacomando[ix] = '.';
                ix++;
                linhacomando[ix] = 'B';
                ix++;
                linhacomando[ix] = 'I';
                ix++;
                linhacomando[ix] = 'N';
                ix++;
                linhacomando[ix] = '\0';

                vretfat = ERRO_D_NOT_FOUND; //fsFindInDir(linhacomando, TYPE_FILE)
                if (vretfat <= ERRO_D_START) {
                    // Se tiver, carrega em 0x01000000 e executa
                    loadFile((unsigned char*)linhacomando, (unsigned char*)vMemUserArea);
                    if (!verro)
                        runCmd();
                    else {
                        printf("Loading File Error...\n");
                    }

                    ix = 255;
                }
                else {
                    // Se nao tiver, mostra erro
                    printf("Invalid Command or File Name\n");
                    ix = 255;
                }*/
            }

            if (ix != 255)  
            {
                if (((vpicret) && (vbytepic != RETURN_OK)) || ((!vpicret) && (vretfat != RETURN_OK))) 
                {
                    printf("Command unsuccessfully\n\0");
                }
                else 
                {
                    if (!strcmp(linhacomando,"CD")) 
                    {
                        if (linhaarg[0] == '.' && linhaarg[1] == '.') 
                        {
                            while (*vdiratup != '/') 
                            {
                                *vdiratup-- = '\0';
                            }

                            if (vdiratup > vdiratu)
                                *vdiratup = '\0';
                            else
                                vdiratup++;
                        }
                        else if(linhaarg[0] == '/') 
                        {
                            vdiratup = vdiratu;
                            *vdiratup++ = '/';
                            *vdiratup = '\0';
                        }
                        else if(linhaarg[0] != '.') 
                        {
                            vdiratup--;
                            if (*vdiratup++ != '/')
                                *vdiratup++ = '/';
                            for (varg = 0; varg < ix; varg++)
                                *vdiratup++ = linhaarg[varg];
                            *vdiratup = '\0';   
                        }
                    }
                    else if (!strcmp(linhacomando,"IFCONFIG")) 
                    {
                        for(iy = 1; iy <= 5; iy++) 
                        {
                            for(ix = 0; ix <= 14; ix++) 
                            {
                                /* ver como fazer */
                                vlinha[ix] = vbytepic;
                            }
                            vlinha[ix] = '\0';
                            switch (iy) 
                            { 
                                case 1:
                                    printf("    IP Addr: %s\n", vlinha);
                                    break;
                                case 2:
                                    printf("    SubMask: %s\n", vlinha);
                                    break;
                                case 3:
                                    printf("    Gateway: %s\n", vlinha);
                                    break;
                                case 4:
                                    printf("   DNS Prim: %s\n", vlinha);
                                    break;
                                case 5:
                                    printf("    DNS Sec: %s\n", vlinha);
                                    break;
                            }
                        }
                    }
/*                    else if (!strcmp(linhacomando,"FORMAT")) 
                    {
                        writes("Format disk was successfully\n\0", vcorf, vcorb);
                    }*/
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void CMMSJOS::putPrompt(unsigned int plinadd) {
    if (plinadd)
        p_mOut->SetRow(p_mOut->GetRow() + 1);

    p_mOut->locate(0,p_mOut->GetRow(), NOREPOS_CURSOR);

    printf("#%s>",vdiratu);

    vinip = p_mOut->GetColumn();

    p_mOut->ativaCursor();
}

//-----------------------------------------------------------------------------
unsigned long CMMSJOS::loadFile(unsigned char *parquivo, unsigned char* xaddress)
{
    unsigned int vsizefile = 0, dd;
    char Buffer[128];
    unsigned nBytesRead;
    FIL File;
    FRESULT Result;

    Result = f_open (&File, retPathAndFile((char*)parquivo), FA_READ | FA_OPEN_EXISTING);

    if (Result != FR_OK)
    {
        printf("File Not Found (%d)\n", Result);
        return 0xFFFFFFFF;
    }

    while ((Result = f_read (&File, Buffer, sizeof Buffer, &nBytesRead)) == FR_OK)
    {
        if (nBytesRead > 0) 
        {
            vsizefile += nBytesRead;
            for (dd = 00; dd <= 127; dd++)
                *xaddress++ = Buffer[dd];
        }

        if (nBytesRead < sizeof Buffer)     // EOF?
            break;
    }

    return vsizefile;
}

//-----------------------------------------------------------------------------
unsigned char CMMSJOS::loadCFG(void) 
{
    unsigned char vret = 1, vset[40], vparam[40], vigual, ipos;
    unsigned int ix, vval;
    unsigned char *mcfgfileptr = (unsigned char*)mcfgfile, varquivo[12];

    strcpy((char*)varquivo,"/sys/cfg/mmsjos.ini");

    if (loadFile(varquivo, (unsigned char*)mcfgfile) > 0) {
        vset[0] = 0x00;
        vparam[0] = 0x00;
        vigual = 0;
        ipos = 0;
        for (ix = 0; ix <= 2047; ix++) {
            if (*mcfgfileptr == '=') {
                vigual = ipos;
                vset[ipos] = 0x00;
                ipos = 0;
            }
            else if (*mcfgfileptr == 0x0D) {
                if (!vigual) {
                    vset[ipos] = 0x00;
                    vigual = ipos;
                }
                else
                    vparam[ipos] = 0x00;

                if (!strcmp((char*)vset,"FCOLOR") && vigual == 6) {
                    if (p_mOut->GetOutput() == 1)
                    {
                        vval = atoi((char*)vparam);
                        vMMSJConfig.TextColorF = vval;
                        p_mOut->SetColorForeground(vval);
                    }
                }
                else if (!strcmp((char*)vset,"BCOLOR") && vigual == 6) {
                    if (p_mOut->GetOutput() == 1)
                    {
                        vval = atoi((char*)vparam);
                        vMMSJConfig.TextColorB = vval;
                        p_mOut->SetColorBackground(vval);
                    }
                }
                else if (!strcmp((char*)vset,"WFCOLOR") && vigual == 7) {
                    if (p_mOut->GetOutput() == 2)
                    {
                        vval = atoi((char*)vparam);
                        vMMSJConfig.MgiColorF = vval;
                        p_mOut->SetColorForeground(vval);
                    }
                }
                else if (!strcmp((char*)vset,"WBCOLOR") && vigual == 7) {
                    if (p_mOut->GetOutput() == 2)
                    {
                        vval = atoi((char*)vparam);
                        vMMSJConfig.MgiColorB = vval;
                        p_mOut->SetColorBackground(vval);
                    }
                }
                else if (!strcmp((char*)vset,"PATH") && vigual == 4) {
                    // futurinho
                }
                else if (!strcmp((char*)vset,"STARTENV") && vigual == 8) {
                    vval = atoi((char*)vparam);

                    if (vval == 0 || vval == 1)
                        vMMSJConfig.StartEnv = vval;
                }
                else if (!strcmp((char*)vset,"TYPECOMM") && vigual == 8) {
                    vval = atoi((char*)vparam);

                    if (vval <= 2)
                        vMMSJConfig.TypeComm = vval;
                }
                else if (!strcmp((char*)vset,"[END]") && vigual == 5) {
                    break;
                }

                vset[0] = 0x00;
                vparam[0] = 0x00;
                ipos = 0;
                vigual = 0;
            }
            else if (*mcfgfileptr >= 0x20 && *mcfgfileptr <= 0x7F) {
                if (!vigual)
                    vset[ipos] = *mcfgfileptr;
                else
                    vparam[ipos] = *mcfgfileptr;

                ipos++;
            }

            if (ipos > 40) {
                printf("Config file syntax error...\n");

                break;
            }

            mcfgfileptr++;
        }

        if (ipos <= 40) {
            printf("Settings applied successfully...\n");
        }

        p_mOut->writec(0x08, NOADD_POS_SCR);
        p_mOut->ativaCursor();
    }
    else
    {
        printf("Loading config file error...\n");
    }

    return vret;
}

//-----------------------------------------------------------------------------
void CMMSJOS::load232(void)
{
    unsigned char pByteComm = 0x00, lResult, vOldLin;
    unsigned int pCount, ix, pSizeSave;
    unsigned long vretfat;
    unsigned int nBytesWritten;
    unsigned char pFileName[15];
    unsigned char *pMemLoad = (unsigned char*)vMemSystemArea;

    printf("Establishing Communication...\n");

    flush_uart();

    while (pByteComm != 0xDD)
        pByteComm = getc();

    pByteComm = getc();

    if (pByteComm == 0xDD)
    {
        printf("Communication Established...\n");

        printf("Receiving File Name...\n");
        pCount = 0;
        lResult = 1;

        while(1)
        {
            pByteComm = 0x00;

            while (pByteComm != 0xDD)
                pByteComm = getc();

            pByteComm = getc();

            pFileName[pCount] = toupper(pByteComm);
            pCount++;

            if (pCount == 15)
                break;
        }

        if (lResult)
        {
            printf("Receiving File %s\n",pFileName);

            vOldLin = p_mOut->GetRow();
            pCount = 0;

            while(1)
            {
                if (pCount % 100 == 0) 
                {
                    p_mOut->locate(0, vOldLin, NOREPOS_CURSOR);
                    printf("Receiving... %d Byte(s)",pCount);
                }

                pByteComm = 0x00;

                while (pByteComm != 0xDD && pByteComm != 0xEE)
                    pByteComm = getc();

                if (pByteComm == 0xDD)
                {
                    pByteComm = getc();

                    *pMemLoad++ = pByteComm;
                    pCount++;
                }
                else if (pByteComm == 0xEE)
                {
                    pByteComm = getc();

                    if (pByteComm == 0xDD)
                    {
                        // Salvar Arquivo
                        p_mOut->locate(0, vOldLin, NOREPOS_CURSOR);
                        printf("Received... %d Byte(s)     ",pCount);
                        printf("\nSaving File...\n");

                        FIL File;
                        if (f_open (&File, retPathAndFile((char*)pFileName), FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
                        {
                            printf("Error Creating File...\n");
                            return;
                        }
                        else
                        {
                            pMemLoad = (unsigned char*)vMemSystemArea;
                            ix = 0;

                            if (pCount > 128)
                                pSizeSave = 128;
                            else
                                pSizeSave = pCount;

                            while(1)
                            {
                                if (f_write (&File, pMemLoad, pSizeSave, &nBytesWritten) != FR_OK)
                                    break;

                                pMemLoad += 128;
                                ix += 128;

                                if (ix >= pCount)
                                    break;

                                if ((pCount - ix) > 128)
                                    pSizeSave = 128;
                                else
                                    pSizeSave = (pCount - ix);
                            }

                            if (f_close (&File) == FR_OK)
                                printf("File Saved...\n");
                            else
                                printf("Erro Saving File (%d)\n",vretfat);
                        }

                        break;
                    }
                }
            }
        }
    }
    else 
        printf("Wrong 2nd.Start (0x%02X) Received.\n", pByteComm);
}

//-----------------------------------------------------------------------------
void CMMSJOS::catFile(unsigned char *parquivo) 
{
    unsigned char *mcfgfileptr = (unsigned char*)mcfgfile, vqtd = 1;
    unsigned char *parqptr = (unsigned char*)parquivo;
    unsigned long vsizefile;

    while (*parqptr++)
        vqtd++;

    vsizefile = loadFile(parquivo, mcfgfileptr);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco

    if (!verro) 
    {
        while (vsizefile > 0) 
        {
            if (*mcfgfileptr == 0x0D) 
            {
                p_mOut->locate(0, p_mOut->GetRow(), NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x0A) 
            {
                p_mOut->SetRow(p_mOut->GetRow() + 1);
                p_mOut->locate(p_mOut->GetColumn(), p_mOut->GetRow(), NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x1A || *mcfgfileptr == 0x00) 
            {
                break;
            }
            else 
            {
                if (*mcfgfileptr >= 0x20 && *mcfgfileptr <= 0x7F)
                    p_mOut->writec(*mcfgfileptr, ADD_POS_SCR);
                else
                    p_mOut->writec(0x20, ADD_POS_SCR);
            }

            mcfgfileptr++;
            vsizefile--;
        }
    }
    else 
    {
        printf("Loading file error...\n");
    }
}

//-----------------------------------------------------------------------------
char * CMMSJOS::_strcat (char *pDest, const char *pSrc)
{
    char *p = pDest;

    while (*p)
    {
        p++;
    }

    while (*pSrc)
    {
        *p++ = *pSrc++;
    }

    *p = '\0';

    return pDest;
}

//-------------------------------------------------------------------------
unsigned int CMMSJOS::bcd2dec(unsigned int bcd)
{
    unsigned int dec=0;
    unsigned int mult;
    for (mult=1; bcd; bcd=bcd>>4,mult*=10)
        dec += (bcd & 0x0f) * mult;
    return dec;
}

//-------------------------------------------------------------------------
unsigned int CMMSJOS::datetimetodir(unsigned char hr_day, unsigned char min_month, unsigned char sec_year, unsigned char vtype)
{
  unsigned int vconv = 0, vtemp;

  if (vtype == CONV_DATA) {
      vtemp = sec_year - 1980;
    vconv  = (unsigned int)(vtemp & 0x7F) << 9;
    vconv |= (unsigned int)(min_month & 0x0F) << 5;
    vconv |= (unsigned int)(hr_day & 0x1F);
  }
  else {
    vconv  = (unsigned int)(hr_day & 0x1F) << 11;
    vconv |= (unsigned int)(min_month & 0x3F) << 5;
    vtemp = sec_year / 2;
    vconv |= (unsigned int)(vtemp & 0x1F);
  }

  return vconv;
}

//-------------------------------------------------------------------------
TCHAR * CMMSJOS::retPathAndFile(char * cFileName)
{
    TCHAR *pPathFileName = (TCHAR *)DRIVE;

    pPathFileName[3] = 0x00;

    if (*cFileName != '/')
    {
        if (strlen((char*)vdiratu) > 1)
        {
            strcat(pPathFileName, (char*)vdiratu);
            strcat(pPathFileName, "/\0");
            strcat(pPathFileName, cFileName);
        }
        else 
        {
            strcat(pPathFileName, (char*)vdiratu);
            strcat(pPathFileName, cFileName);
        }
    }
    else
        strcat(pPathFileName, cFileName);

    return pPathFileName;
}

//-------------------------------------------------------------------------
// USB Keyboard Routine
//-------------------------------------------------------------------------
void CMMSJOS::KeyPressedHandler (const char *pString)
{
    int ikx, iksz;
    assert (s_pThis != 0);

    iksz = strlen(vkeybuffer);

    for (ikx = iksz; ikx <= strlen(pString); ikx++)
    {
        vkeybuffer[ikx] = *pString++;
    }
}

//-------------------------------------------------------------------------
void CMMSJOS::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
    assert (s_pThis != 0);

    CString sMsgs;
    sMsgs.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);

    for (unsigned i = 0; i < 6; i++)
    {
        if (RawKeys[i] != 0)
        {
            CString KeyCode;
            KeyCode.Format (" %02X", (unsigned) RawKeys[i]);

            sMsgs.Append (KeyCode);
        }
    }

    CLogger::Get ()->Write("OS", LogNotice, sMsgs);
}

//
// Basic File System Access
//
int CMMSJOS::fsMount(void)
{
    FRESULT pResM;

    pResM = f_mount (p_mFileSystem, DRIVE, 1);
    if (pResM != FR_OK)
    {
        CLogger::Get ()->Write("OS", LogNotice, "Cannot mount drive: %s", DRIVE);
    }

    return pResM;
}

//-----------------------------------------------------------------------------
// Parte Grafica
//-----------------------------------------------------------------------------
void CMMSJOS::startMGI(void) 
{
    unsigned char vnomefile[12];
    unsigned char ix, lc, ll, *ptr_ico, *ptr_prg, *ptr_pos;
    unsigned char* vMemSystemAreaPos;

    p_mOut->desativaCursor();

    ptr_pos = (unsigned char*)(vMemSystemArea + (MEM_POS_MGICFG + 16));
    ptr_ico = ptr_pos + 32;
    ptr_prg = ptr_ico + 320;

    for (lc = 0; lc <= 31; lc++) {
        *ptr_pos++ = 0x00;
        for (ll = 0; ll <= 9; ll++) {
            *ptr_ico++ = 0x00;
            *ptr_prg++ = 0x00;
        }
    }

    vkeyopen = 0;
    vTimerShowed = 0;

    printf("\nLoading Config...\n");
    vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_MGICFG);
    vnomefile[0] = 0x00;
    _strcat((char*)vnomefile,(char*)"/sys/cfg/MMSJMGI");
    _strcat((char*)vnomefile,(char*)".CFG");
    loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

    printf("Loading Icons...\n");
    vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_ICONES);

    for (ix = 0; ix <= 9; ix++)
    {
        vnomefile[0] = 0x00;
        _strcat((char*)vnomefile,(char*)"/sys/img/");
        _strcat((char*)vnomefile,(char*)pIconsMgi[ix]);
        loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);
        vMemSystemAreaPos += 3262;
    }

    p_mOut->SetOutput(2); // Grafico
    loadCFG();
    redrawMain();

    p_mTimer->StartKernelTimer (60 * HZ, TimerHandler);

    while(editortela());

    p_mOut->SetOutput(1); // Texto
    p_mOut->SetColumn(0);
    p_mOut->SetRow(0);

    p_mOut->clearScr(p_mOut->GetColorBackground());

    p_mOut->ativaCursor();
}

//-----------------------------------------------------------------------------
void CMMSJOS::redrawMain(void) 
{
    p_mOut->clearScr(Black);
    loadImage(0,0,320,240,(char*)"/sys/img/walpaper.bmp",IMGFORMATBMP);

    p_mOut->SaveScreen(0,0,320,240);

    // Desenhar Barra Menu Principal / Status
    desenhaMenu();

    // Desenhar Icones da tela (lendo do disco)
    //desenhaIconesUsuario();
}

//-----------------------------------------------------------------------------
void CMMSJOS::redrawMainRest(void) 
{
    p_mOut->RestoreScreen(0,0,320,240);
    p_mOut->SaveScreen(0,0,320,240);

    // Desenhar Barra Menu Principal / Status
    desenhaMenu();

    // Desenhar Icones da tela (lendo do disco)
    //desenhaIconesUsuario();
}

//-----------------------------------------------------------------------------
void CMMSJOS::desenhaMenu(void) 
{
    unsigned int vx, vy;

    vx = COLMENU;
    vy = LINMENU;

    p_mOut->FillRect(0, 0, p_mOut->GetWidth(), 34, Black);

    MostraIcone(vx, vy, 50);    // Home

    desenhaTimer();
}

//-----------------------------------------------------------------------------
void CMMSJOS::desenhaTimer(void)
{
    unsigned char lc;
    char vTimeTotal[16], vDateW[11], vTimeW[6];
    CString *pTimeString = p_mTimer->GetDateTimeString();

    strcpy(vTimeTotal,pTimeString->GetString());

    for (lc = 0; lc <= 9; lc++)
        vDateW[lc] = vTimeTotal[lc];
    vDateW[10] = 0x00;

    for (lc = 0; lc <= 4; lc++)
        vTimeW[lc] = vTimeTotal[lc + 11];
    vTimeW[5] = 0x00;

    p_mOut->writesxy(256,5,8,vTimeW,White,Black);
    p_mOut->writesxy(240,21,8,vDateW,White,Black);

    vTimerShowed = 1;
}

//-----------------------------------------------------------------------------
void CMMSJOS::desenhaIconesUsuario(void) {
  unsigned int vx, vy;
  unsigned char lc, *ptr_ico, *ptr_prg, *ptr_pos;

  // COLOCAR ICONSPERLINE = 10
  // COLOCAR SPACEICONS = 8
  
  next_pos = 0;

  ptr_pos = (unsigned char*)(vMemSystemArea + (MEM_POS_MGICFG + 16));
  ptr_ico = ptr_pos + 32;
  ptr_prg = ptr_ico + 320;

  for(lc = 0; lc <= (ICONSPERLINE * 4 - 1); lc++) {
    ptr_pos = ptr_pos + lc;
    ptr_ico = ptr_ico + (lc * 10);
    ptr_prg = ptr_prg + (lc * 10);

    if (*ptr_prg != 0 && *ptr_ico != 0) {
      if (*ptr_pos <= (ICONSPERLINE - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * *ptr_pos;
        vy = 40;
      }
      else if (*ptr_pos <= (ICONSPERLINE * 2 - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE);
        vy = 72;
      }
      else if (*ptr_pos <= (ICONSPERLINE * 3 - 1)) {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE);
        vy = 104;
      }
      else {
        vx = COLINIICONS + (24 + SPACEICONS) * (*ptr_pos - ICONSPERLINE * 2);
        vy = 136;
      }

      MostraIcone(vx, vy, lc);

      next_pos = next_pos + 1;
    }
  }
}

//-----------------------------------------------------------------------------
void CMMSJOS::MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone) {
    unsigned char vnomefile[12];
    unsigned char *ptr_prg;
    unsigned char *ptr_viconef;

    ptr_prg = (unsigned char*)(vMemSystemArea + (MEM_POS_MGICFG + 16) + 32 + 320);
    ptr_viconef = (unsigned char*)(vMemSystemArea + (MEM_POS_ICONES + (3262 * (vicone + 10))));

    // Procura Icone no Disco se Nao for Padrao
    if (vicone < 50) {
        ptr_prg = ptr_prg + (vicone * 10);
        vnomefile[0] = 0x00;
        _strcat((char*)vnomefile,(char*)"/usr/img/");
        _strcat((char*)vnomefile,(char*)ptr_prg);
        _strcat((char*)vnomefile,(char*)".ico");
        loadFile(vnomefile, (unsigned char*)ptr_viconef);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
        if (verro)
            vicone = 255;
    }

    if (vicone >= 50) {
        vicone -= 50;
        ptr_viconef = (unsigned char*)(vMemSystemArea + (MEM_POS_ICONES + (3262 * vicone)));
    }

    // Mostra Icone
    PutIcone(ptr_viconef, vvx, vvy);
}

//-----------------------------------------------------------------------------
void CMMSJOS::PutIcone(unsigned char* vimage, unsigned int x, unsigned int y) 
{
    unsigned int pw = 32, ph = 32;

    p_mOut->showImageICO(x, y, pw, ph, (unsigned char*)vimage);
}

//--------------------------------------------------------------------------
void CMMSJOS::TimerHandler (unsigned hTimer, void *pParam, void *pContext)
{
    s_pThis->desenhaTimer();

    s_pThis->p_mTimer->StartKernelTimer (60 * HZ, TimerHandler);
}

//--------------------------------------------------------------------------
unsigned char CMMSJOS::editortela(void) {
    unsigned char vresp = 1;
    unsigned char vx, cc, vpos, vposiconx, vposicony;
    unsigned char *ptr_prg;

    p_mOut->VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

    if (vposty <= 34)
        vresp = showMenu();
    else {
        vposiconx = COLINIICONS;
        vposicony = 40;
        vpos = 0;

        if (vposty >= 136) {
            vpos = ICONSPERLINE * 3;
            vposicony = 136;
        }
        else if (vposty >= 30) {
            vpos = ICONSPERLINE * 2;
            vposicony = 104;
        }
        else if (vposty >= 30) {
            vpos = ICONSPERLINE;
            vposicony = 72;
        }

        if (vpostx >= COLINIICONS && vpostx <= (COLINIICONS + (24 + SPACEICONS) * ICONSPERLINE) && vposty >= 40) {
          cc = 1;
          for(vx = (COLINIICONS + (24 + SPACEICONS) * (ICONSPERLINE - 1)); vx >= (COLINIICONS + (24 + SPACEICONS)); vx -= (24 + SPACEICONS)) {
            if (vpostx >= vx) {
              vpos += ICONSPERLINE - cc;
              vposiconx = vx;
              break;
            }

            cc++;
          }

          ptr_prg = (unsigned char*)(vMemSystemArea + (MEM_POS_MGICFG + 16) + 32 + 320);
          ptr_prg = ptr_prg + (vpos * 10);

          if (*ptr_prg != 0) {
            p_mOut->InvertRect( vposiconx, vposicony, 24, 24);

            strcpy((char*)vbuf,(char *)ptr_prg);

            MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

            processCmd();

            *vbuf = 0x00;

            redrawMainRest();
          }
        }
    }

    return vresp;
}

//-------------------------------------------------------------------------
unsigned char CMMSJOS::showMenu(void) 
{
    unsigned int lc, vposicony, mx, my;
    unsigned char vpos = 0, vresp, ix, iy;
    unsigned int vwidthmenu = (40 * QTDCOLSMENU), vhightmenu = (40 * QTDLINSMENU);
    vresp = 1;

    if (vpostx >= COLMENU && vpostx <= (COLMENU + 32)) 
    {
        mx = 0;
        my = LINHAMENU;

        p_mOut->SaveScreen(0, LINHAMENU, vwidthmenu + 5, vhightmenu + 5);
    
        p_mOut->FillRect(0, LINHAMENU, vwidthmenu, vhightmenu, White);

        lc = 0;
        for(iy = 0; iy < QTDLINSMENU; iy++)
        {
            for(ix = 0; ix < QTDCOLSMENU; ix++)
            {
                mx = ((ix * 40) + 4);
                my = ((iy * 40 ) + LINHAMENU + 4);
                MostraIcone(mx, my, pItensMenuHome[lc]);
                lc++;
            }
        }

        while (1)
        {
            p_mOut->VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

            vpos = 0;
            vposicony = 0;

            for(iy = 0; iy < QTDLINSMENU; iy++)
            {
                for(ix = 0; ix < QTDCOLSMENU; ix++)
                {
                    if ((vposty >= ((iy * 40) + LINHAMENU) && vposty <= (((iy + 1) * 40) + LINHAMENU)) && (vpostx >= (ix * 40) && vpostx <= ((ix + 1) * 40)))
                    {
                        vposicony = 1;
                        break;
                    }
                    vpos++;
                }

                if (vposicony)
                    break;
            }

            p_mOut->RestoreScreen(0, LINHAMENU, vwidthmenu + 5, vhightmenu + 5);    

            if (vposicony > 0)
            {
                switch (vpos) 
                {
                    case 0: // Run
                        break;
                    case 1: // mmsjos
                        break;
                    case 2: // files
                        break;
                    case 3: // setup
                        setupMGI();
                        break;
                    case 4: // Trash
                        break;
                    case 5: // Help
                        message((char*)"MGI v0.1\nGraphical Interface\n \nwww.utilityinf.com.br\0", BTCLOSE, 0);
                        break;
                }
            }
            else 
                break;
        }
    }
    else if (vpostx >= 240) 
    {
        setupDateTimer();
    }

    return vresp;
}

//-------------------------------------------------------------------------
void CMMSJOS::loadImage(unsigned int px, unsigned int py, unsigned int pw, unsigned int ph, char *filename, char pType) 
{
    unsigned long imgSize = loadFile((unsigned char*)filename, (unsigned char*)mcfgfile);

    if (imgSize > 0)
    {
        switch (pType)
        {
            case 0: // BMP
                p_mOut->showImageBMP(px, py, pw, ph, (unsigned char*)mcfgfile);
                break;
            case 1: // ICO
                p_mOut->showImageICO(px, py, pw, ph, (unsigned char*)mcfgfile);
                break;
            default:
                printf("Type Image Not Supported\n");
        }
    }
    else
        printf("Load File %s Error\n",filename);
}

//-------------------------------------------------------------------------
void CMMSJOS::executeCmd(void) {
/*    unsigned char vstring[64], vwb;

    vstring[0] = '\0';

    strcpy((char *)vparamstr,(char*)"Execute");
    vparam[0] = 10;
    vparam[1] = 40;
    vparam[2] = 280;
    vparam[3] = 50;
    vparam[4] = BTOK | BTCANCEL;
    showWindow();

    writesxy(12,55,8,(char*)"Execute:",vcorwf,vcorwb);
    fillin(vstring, 84, 55, 160, WINDISP);

    while (1) {
        fillin(vstring, 84, 55, 160, WINOPER);

        vwb = waitButton();

        if (vwb == BTOK || vwb == BTCANCEL)
            break;
    }

    if (vwb == BTOK) {
        strcpy((char *)p_mMMSJOS->vbuf, (char *)vstring);

        MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

        // Chama processador de comandos
        p_mMMSJOS->processCmd();

        while (vxmaxold != 0) {
            vwb = waitButton();

            if (vwb == BTCLOSE)
                break;
        }

        if (vxmaxold != 0) {
            p_mMMSJOS->vxmax = vxmaxold;
            p_mMMSJOS->vymax = vymaxold;
            p_mMMSJOS->vcol = 0;
            p_mMMSJOS->vlin = 0;
            voverx = 0;
            vovery = 0;
            vxmaxold = 0;
            vymaxold = 0;
        }

        *p_mMMSJOS->vbuf = 0x00;  // Zera Buffer do teclado
    }*/
}

//-------------------------------------------------------------------------
unsigned char CMMSJOS::message(char* bstr, unsigned char bbutton, unsigned int btime)
{
    unsigned int i, ii = 0, xi, yi, xm, ym, pwidth, pheight, xib, yib;
    unsigned char qtdnl, maxlenstr;
    unsigned char qtdcstr[8], poscstr[8];
    unsigned char *bstrptr = new unsigned char[80], *bstrptr2 = new unsigned char[80], *bstrptr3 = bstrptr2;
    unsigned char vBtOk;

    qtdnl = 1;
    maxlenstr = 0;
    qtdcstr[1] = 0;
    poscstr[1] = 0;
    i = 0;

    for (ii = 0; ii <= 7; ii++)
        vbuttonwin[ii] = 0;

    bstrptr = (unsigned char*)bstr;
    while (*bstrptr)
    {
        qtdcstr[qtdnl]++;

        if (qtdcstr[qtdnl] > 26)
            qtdcstr[qtdnl] = 26;

        if (qtdcstr[qtdnl] > maxlenstr)
            maxlenstr = qtdcstr[qtdnl];

        if (*bstrptr == '\n')
        {
            qtdcstr[qtdnl]--;
            qtdnl++;

            if (qtdnl > 6)
                qtdnl = 6;

            qtdcstr[qtdnl] = 0;
            poscstr[qtdnl] = i + 1;
        }

        bstrptr++;
        i++;
    }

    if (maxlenstr > 26)
        maxlenstr = 26;

    if (qtdnl > 6)
        qtdnl = 6;

    pwidth = maxlenstr * 10;
    pwidth = pwidth + 2;
    xm = pwidth / 2;
    xi = 160 - xm - 1;

    pheight = 10 * qtdnl;
    pheight = pheight + 20;
    ym = pheight / 2;
    yi = 120 - ym - 1;

    CWindow *wMessage = new CWindow(p_mOut, (char*)"\0", xi, yi, pwidth, pheight, p_mOut->GetColorForeground(), p_mOut->GetColorBackground());

    for (i = 1; i <= qtdnl; i++)
    {
        xib = xi + xm;
        xib = xib - ((qtdcstr[i] * 8) / 2);
        yib = yi + 2 + (10 * (i - 1));

        bstrptr = (unsigned char*)bstr + poscstr[i];
        bstrptr2 = bstrptr3;
        for (ii = poscstr[i]; ii <= (poscstr[i] + qtdcstr[i] - 1) ; ii++)
            *bstrptr2++ = *bstrptr++;
        *bstrptr2 = 0x00;
        wMessage->addElement(GUITEXT,(xib - xi),(yib - yi),0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)bstrptr3,0);
    }

    vBtOk = wMessage->addElement(GUIBUTTON,2,(pheight - 12),42,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"OK\0",0);  

    ii = wMessage->run(btime, 1, vBtOk);

    delete wMessage;

    return ii;
}

//-------------------------------------------------------------------------
void CMMSJOS::setupMGI(void) 
{
    unsigned char *vopc = new unsigned char[1], vwb;
    unsigned char weRsTypeComm, weRsStartEnv, vBtOk, vBtCanc;

    CWindow *wSetupMgi = new CWindow(p_mOut, (char*)"Configuration\0", 60, 50, 200, 180, p_mOut->GetColorForeground(), p_mOut->GetColorBackground());
    wSetupMgi->addElement(GUITEXT,6,14,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Type Comm.:\0",0);    
    weRsTypeComm = wSetupMgi->addElement(GUIRADIOSET,26,26,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)",BLUETOOTH HM10,BLUETOOTH,SERIAL\0",1,vMMSJConfig.TypeComm);    
    wSetupMgi->addElement(GUITEXT,6,66,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Cores:\0",0);    
    wSetupMgi->addElement(GUITEXT,6,76,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Foreground:\0",0);    
    wSetupMgi->addElement(GUITEXT,6,96,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Background:\0",0);    
    wSetupMgi->addElement(GUITEXT,6,126,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Start Enviroment:\0",0);    
    weRsStartEnv = wSetupMgi->addElement(GUIRADIOSET,26,136,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)",Texto,Grafico (MGI)\0",1,vMMSJConfig.StartEnv);    
    vBtOk = wSetupMgi->addElement(GUIBUTTON,2,168,50,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"OK\0",0);    
    vBtCanc = wSetupMgi->addElement(GUIBUTTON,54,168,50,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Cancel\0",0);    
    vwb = wSetupMgi->run(0, 2, vBtOk, vBtCanc);

    if (vwb == vBtOk) 
    {
        MostraIcone(160, 120, ICON_HOURGLASS);  // Mostra Amplulheta

        vopc = wSetupMgi->GetReturnElement(weRsTypeComm);
        vMMSJConfig.TypeComm = vopc[0];
        vopc = wSetupMgi->GetReturnElement(weRsStartEnv);
        vMMSJConfig.StartEnv = vopc[0];

        // Grava no Arquivo
    }

    delete wSetupMgi;
}

//-------------------------------------------------------------------------
void CMMSJOS::setupDateTimer(void)
{
    unsigned char vwb, vBtOk, vBtCanc;
    unsigned char vdayw, vmontw, vyearw, vhourw, vminw;
    unsigned char strDay[3], strMon[3], strYear[5];
    unsigned char strHrs[3], strMin[3];
    unsigned char *strPtr = new unsigned char[3];
    unsigned long pRetWriteData;
    time_t pData;
    unsigned nTime = 0;

    pData = ds1307_read();

    if (pData.Error) 
        message((char*)"Read RTC Error\0", BTCLOSE, 0);
    else
    {
        strDay[0] = BCD2UpperCh(pData.Day);
        strDay[1] = BCD2LowerCh(pData.Day);
        strDay[2] = 0;
        strMon[0] = BCD2UpperCh(pData.Month);
        strMon[1] = BCD2LowerCh(pData.Month);
        strMon[2] = 0;
        strYear[0] = '2';
        strYear[1] = '0';
        strYear[2] = BCD2UpperCh(pData.Year);
        strYear[3] = BCD2LowerCh(pData.Year);
        strYear[4] = 0;

        strHrs[0] = BCD2UpperCh(pData.Hour);
        strHrs[1] = BCD2LowerCh(pData.Hour);
        strHrs[2] = 0;
        strMin[0] = BCD2UpperCh(pData.Minute);
        strMin[1] = BCD2LowerCh(pData.Minute);
        strMin[2] = 0;

        CWindow *wSetupDT = new CWindow(p_mOut, (char*)"Date and Time\0", 40, 60, 200, 100, p_mOut->GetColorForeground(), p_mOut->GetColorBackground());
        wSetupDT->addElement(GUITEXT,6,14,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Date:\0",0);    
        vdayw = wSetupDT->addElement(GUIFILLIN,10,24,20,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)strDay,0);    
        wSetupDT->addElement(GUISPIN,35,22,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"\0",3,vdayw,1,31);    
        vmontw = wSetupDT->addElement(GUIFILLIN,50,24,20,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)strMon,0);    
        wSetupDT->addElement(GUISPIN,75,22,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"\0",3,vmontw,1,12);    
        vyearw = wSetupDT->addElement(GUIFILLIN,90,24,40,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)strYear,0);    
        wSetupDT->addElement(GUISPIN,135,22,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"\0",3,vyearw,0,9999);    
        wSetupDT->addElement(GUITEXT,6,44,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Time:\0",0);    
        vhourw = wSetupDT->addElement(GUIFILLIN,10,54,20,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)strHrs,0);    
        wSetupDT->addElement(GUISPIN,35,52,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"\0",3,vhourw,0,23);    
        vminw = wSetupDT->addElement(GUIFILLIN,50,54,20,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)strMin,0);    
        wSetupDT->addElement(GUISPIN,75,52,0,0, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"\0",3,vminw,0,59);    
        vBtOk = wSetupDT->addElement(GUIBUTTON,2,88,50,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"OK\0",0);    
        vBtCanc = wSetupDT->addElement(GUIBUTTON,54,88,50,10, p_mOut->GetColorForeground(), p_mOut->GetColorBackground(),(unsigned char*)"Cancel\0",0);    
        vwb = wSetupDT->run(0, 2, vBtOk, vBtCanc);

        if (vwb == vBtOk)
        {
            MostraIcone(160, 120, ICON_HOURGLASS);  // Mostra Amplulheta

            strPtr = wSetupDT->GetReturnElement(vdayw);
            pData.Day = atoi((char*)strPtr);
            strPtr = wSetupDT->GetReturnElement(vmontw);
            pData.Month = atoi((char*)strPtr);
            strPtr = wSetupDT->GetReturnElement(vyearw);
            pData.Year = (atoi((char*)strPtr) - 2000);

            strPtr = wSetupDT->GetReturnElement(vhourw);
            pData.Hour = atoi((char*)strPtr);
            strPtr = wSetupDT->GetReturnElement(vminw);
            pData.Minute = atoi((char*)strPtr);
            pData.Second = 0;

            pRetWriteData = ds1307_write(pData);

            if (pRetWriteData) 
                message((char*)"Write RTC Error\0", BTCLOSE, 0);

            pData = ds1307_read();

            if (!pData.Error)
            {
                for (unsigned nYear = 1970; nYear < (unsigned)bcd2bin(pData.Year)+2000; nYear++)
                {
                    nTime += p_mTimer->IsLeapYear (nYear) ? 366 : 365;
                }

                for (unsigned nMonth = 0; nMonth < (unsigned)(bcd2bin(pData.Month) - 1); nMonth++)
                {
                    nTime += p_mTimer->GetDaysOfMonth (nMonth, bcd2bin(pData.Year)+2000);
                }

                nTime += bcd2bin(pData.Day)-1;
                nTime *= 24;
                nTime += bcd2bin(pData.Hour);
                nTime *= 60;
                nTime += bcd2bin(pData.Minute);
                nTime *= 60;
                nTime += bcd2bin(pData.Second);

                p_mTimer->SetTime (nTime, FALSE);            
            }
            else
                message((char*)"Read RTC Error\0", BTCLOSE, 0);

            desenhaTimer();
        }

        delete wSetupDT;
    }
}
