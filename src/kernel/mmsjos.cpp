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
* 52 = New Icon
* 53 = Del Icon
* 54 = MMSJDOS
* 55 = Setup MGI
* 56 = Exit
* 57 = Hourglass
*
********************************************************************************/

#define __USE_TFT_SCROLL__

#include <circle/usb/usbkeyboard.h>
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
        pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
    }

    pResM = fsMount();
    if (pResM == FR_OK)
        printf("OS> Mounting Disk... Done\n");
    else
        printf("OS> Mounting Disk... Error (%d)\n",pResM);

    return TRUE;
}

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void CMMSJOS::Start(void)
{
    int ikx;

    // Inicio SO
    strcpy((char*)vdiratu,"/\0");

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
            pKeyboard->UpdateLEDs ();

            if (vkeybuffer[0] != 0)
            {
                vbytepic = vkeybuffer[0];

                for(ikx = 0; ikx < strlen(vkeybuffer); ikx++)
                    vkeybuffer[ikx] = vkeybuffer[ikx + 1];
                vkeybuffer[ikx] = 0x00;
            }
            else
                vbytepic = 0x00;
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
        if (!strcmp(linhacomando,"CLS") && iy == 3) 
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
            loadImage(0,0,320,240,(char*)vparam);
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
                loadCFG(1);
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
unsigned char CMMSJOS::loadCFG(unsigned char ptipo) {
    unsigned char vret = 1, vset[40], vparam[40], vigual, ipos;
    unsigned int ix, vval;
    unsigned char *mcfgfileptr = (unsigned char*)mcfgfile, varquivo[12];

    strcpy((char*)varquivo,"MMSJCFG.INI");
    loadFile(varquivo, (unsigned char*)&mcfgfile);

    if (!verro) {
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
                    vval = atoi((char*)vparam);
                    p_mOut->SetColorForeground(vval);
                }
                else if (!strcmp((char*)vset,"BCOLOR") && vigual == 6) {
                    vval = atoi((char*)vparam);
                    p_mOut->SetColorBackground(vval);
                }
                else if (!strcmp((char*)vset,"PATH") && vigual == 4) {
                    // futurinho
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
                if (ptipo) {
                    printf("Config file syntax error...\n");
                }

                break;
            }

            mcfgfileptr++;
        }

        if (ipos <= 40 && ptipo) {
            printf("Settings applied successfully...\n");
        }

        p_mOut->writec(0x08, NOADD_POS_SCR);
        p_mOut->ativaCursor();
    }
    else {
        if (ptipo) {
            printf("Loading config file error...\n");
        }
        vret = 0;
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

    CString Message;
    Message.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);

    for (unsigned i = 0; i < 6; i++)
    {
        if (RawKeys[i] != 0)
        {
            CString KeyCode;
            KeyCode.Format (" %02X", (unsigned) RawKeys[i]);

            Message.Append (KeyCode);
        }
    }

    CLogger::Get ()->Write("OS", LogNotice, Message);
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
    unsigned char lc, ll, *ptr_ico, *ptr_prg, *ptr_pos;
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
    p_mOut->SetOutput(2); // Grafico
    p_mOut->SetTypeKeyboard((pKeyboard == 0 ? 0 : 1)); // 0 - Touch, 1 - USB

    p_mOut->SetColorForeground(White);
    p_mOut->SetColorBackground(Blue);

    vparamstr[0] = '\0';
    vparam[0] = 20;
    vparam[1] = 80;
    vparam[2] = 280;
    vparam[3] = 100;
    vparam[4] = BTNONE;
    p_mOut->showWindow(vparamstr, vparam);

    p_mOut->writesxy(140,85,16,(char*)"MGI",p_mOut->GetColorForeground(),p_mOut->GetColorBackground());
    p_mOut->writesxy(74,105,8,(char*)"Graphical Interface",p_mOut->GetColorForeground(),p_mOut->GetColorBackground());
    p_mOut->writesxy(94,166,8,(char*)"Please Wait...",p_mOut->GetColorForeground(),p_mOut->GetColorBackground());

    p_mOut->writesxy(86,155,8,(char*)"Loading Config",p_mOut->GetColorForeground(),p_mOut->GetColorBackground());
    vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_MGICFG);
    vnomefile[0] = 0x00;
    _strcat((char*)vnomefile,(char*)"/sys/cfg/MMSJMGI");
    _strcat((char*)vnomefile,(char*)".CFG");
    loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

    p_mOut->writesxy(86,155,8,(char*)"Loading Icons ",p_mOut->GetColorForeground(),p_mOut->GetColorBackground());
    vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_ICONES);
    vnomefile[0] = 0x00;
    _strcat((char*)vnomefile,(char*)"/sys/img/MOREICON");
    _strcat((char*)vnomefile,(char*)".LIB");
    loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

    p_mTimer->MsDelay (1000);

    redrawMain();

    p_mOut->SetColorForeground(White);
    p_mOut->SetColorBackground(Black);

    while(editortela());

    p_mOut->SetOutput(1); // Texto
    p_mOut->SetColumn(0);
    p_mOut->SetRow(0);

    p_mOut->clearScr(p_mOut->GetColorBackground());

    p_mOut->ativaCursor();
}

//-----------------------------------------------------------------------------
void CMMSJOS::PutIcone(unsigned char* vimage, unsigned int x, unsigned int y) 
{
    unsigned int ix, pw, ph;
    unsigned int pimage[640];

    for (ix = 0; ix <= 575; ix++)
    {
        pimage[ix] = ((*vimage++ & 0xFF) << 8); 
        pimage[ix] += (*vimage++ & 0xFF);
    }

    pw = 24;
    ph = 24;

    p_mOut->PutImage(pimage, x, y, pw, ph);
}

//-----------------------------------------------------------------------------
void CMMSJOS::redrawMain(void) 
{
    p_mOut->clearScr(Black);
    loadImage(0,0,320,240,(char*)"/sys/img/ut_logo.bmp");

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
    unsigned char lc;
    unsigned int vx, vy;

    vx = COLMENU;
    vy = LINMENU;

    p_mOut->FillRect(0, 0, p_mOut->GetWidth(), 35, Black);

    for (lc = 50; lc <= 56; lc++) 
    {
        MostraIcone(vx, vy, lc);
        vx += 32;
    }

    p_mOut->FillRect(0, (p_mOut->GetHeight() - 35), p_mOut->GetWidth(), 35, Black);
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
    ptr_viconef = (unsigned char*)(vMemSystemArea + (MEM_POS_ICONES + (1152 * (vicone + 10))));

    // Procura Icone no Disco se Nao for Padrao
    if (vicone < 50) {
        ptr_prg = ptr_prg + (vicone * 10);
        vnomefile[0] = 0x00;
        _strcat((char*)vnomefile,(char*)"/usr/img/");
        _strcat((char*)vnomefile,(char*)ptr_prg);
        _strcat((char*)vnomefile,(char*)".ICO");
        loadFile(vnomefile, (unsigned char*)&ptr_viconef);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
        if (verro)
            vicone = 59;
    }

    if (vicone >= 50) {
        vicone -= 50;
        ptr_viconef = (unsigned char*)(vMemSystemArea + (MEM_POS_ICONES + (1152 * vicone)));
    }

    // Mostra Icone
    PutIcone(ptr_viconef, vvx, vvy);
}

//--------------------------------------------------------------------------
unsigned char CMMSJOS::editortela(void) {
    unsigned char vresp = 1;
    unsigned char vx, cc, vpos, vposiconx, vposicony;
    unsigned char *ptr_prg;

    p_mOut->VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

    // Para Testes
//    p_mOut->SetColumn(0);
//    p_mOut->SetRow(10);
//    printf("%d x %d",vpostx,vposty);

    if (vposty <= 30)
        vresp = new_menu();
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
unsigned char CMMSJOS::new_menu(void) {
    unsigned int vy, lc, vposicony, mx, my, menyi[8], menyf[8];
    unsigned char vpos = 0, vresp, mpos;

    vresp = 1;

    if (vpostx >= COLMENU && vpostx <= (COLMENU + 24)) {
        mx = 0;
        my = LINHAMENU;
        mpos = 0;

        p_mOut->FillRect(mx,my,128,42,White);
        p_mOut->DrawRect(mx,my,128,42,Black);

        mpos += 2;
        menyi[0] = my + mpos;
        p_mOut->writesxy(mx + 8,my + mpos,8,(char*)"Format",Black,White);
        mpos += 12;
        menyf[0] = my + mpos;
        p_mOut->DrawLine(mx,my + mpos,mx+128,my + mpos,Black);

        mpos += 2;
        menyi[1] = my + mpos;
        p_mOut->writesxy(mx + 8,my + mpos,8,(char*)"Help",Black,White);
        mpos += 12;
        menyf[1] = my + mpos;
        mpos += 2;
        menyi[2] = my + mpos;
        p_mOut->writesxy(mx + 8,my + mpos,8,(char*)"About",Black,White);
        mpos += 12;
        menyf[2] = my + mpos;
        p_mOut->DrawLine(mx,my + mpos,mx+128,my + mpos,Black);

        p_mOut->VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

        if ((vposty >= my && vposty <= my + 42) && (vpostx >= mx && vpostx <= mx + 128)) {
            vpos = 0;
            vposicony = 0;

            for(vy = 0; vy <= 1; vy++) {
                if (vposty >= menyi[vy] && vposty <= menyf[vy]) {
                    vposicony = menyi[vy];
                    break;
                }

                vpos++;
            }

            if (vposicony > 0)
                p_mOut->InvertRect( mx + 4, vposicony, 120, 12);

            switch (vpos) {
                case 0: // Format
                    break;
                case 1: // Help
                    break;
                case 2: // About
                    p_mOut->message((char*)"MGI v0.1\nGraphical Interface\n \nwww.utilityinf.com.br\0", BTCLOSE, 0);
                    break;
            }
        }

        redrawMainRest();
    }
    else {
        for (lc = 1; lc <= 6; lc++) {
            mx = COLMENU + (32 * lc);
            if (vpostx >= mx && vpostx <= (mx + 24)) {
                p_mOut->InvertRect( mx, 4, 24, 24);
                p_mOut->InvertRect( mx, 4, 24, 24);
                break;
            }
        }

        switch (lc) {
            case 1: // RUN
                executeCmd();
                break;
            case 2: // NEW ICON
                break;
            case 3: // DEL ICON
                break;
            case 4: // MMSJDOS
                strcpy((char *)vbuf,(char*)"MDOS\0");

                MostraIcone(144, 104, ICON_HOURGLASS);

                processCmd();

                *vbuf = 0x00;

                break;
            case 5: // SETUP
                break;
            case 6: // EXIT
                mpos = p_mOut->message((char*)"Deseja Sair ?\0", BTYES | BTNO, 0);
                if (mpos == BTYES)
                    vresp = 0;

                break;
        }

        if (lc < 6)
            redrawMainRest();
    }

    return vresp;
}

//-------------------------------------------------------------------------
void CMMSJOS::loadImage(unsigned int px, unsigned int py, unsigned int pw, unsigned int ph, char *filename) 
{
    unsigned long imgSize = loadFile((unsigned char*)filename, (unsigned char*)mcfgfile);
    if (imgSize > 0)
    {
        printf("%08X\n", mcfgfile);
        printf("%02X.%02X.%02X.%02X\n", *(mcfgfile + 4), *(mcfgfile + 5), *(mcfgfile + 6), *(mcfgfile + 7));
        p_mOut->showImageBMP(px, py, pw, ph, (unsigned char*)mcfgfile);
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
void CMMSJOS::new_icon(void) {
/*
  byte vx, vy, cc, verro, vwb;

  icon_ico[next_pos][0] = '\0';

  vstring[0] = '\0';
  SaveScreen(4,3,121,50);
  showWindow("New Icon\0", 4, 3, 121, 50, BTOK | BTCANCEL);
  GotoXY(6,14);
  writes("Program Name:");
  fillin(vstring, 6, 24, 113, WINDISP);

  vwaittouch = 1;
  while (1) {
    fillin(vstring, 6, 24, 113, WINOPER);

    vwb = waitButton();

    if (vwb == BTOK || vwb == BTCANCEL)
      break;
  }

  RestoreScreen(4,3,121,50);

  if (vwb == BTOK) {
    vkeybufflen = _strlen(vstring);
    _strcpy(vkeybuff, vstring);

    for (cc = 0; cc <= vkeybufflen; cc++) {
      if (vkeybuff[cc] == '\0')
        break;

      icon_ico[next_pos][cc] = toupper(vkeybuff[cc]);
      icon_ico[next_pos][cc + 1] = '\0';
      icon_prg[next_pos][cc] = toupper(vkeybuff[cc]);
      icon_prg[next_pos][cc + 1] = '\0';
    }

    verro = 0;

    vkeybufflen = 0;
    vkeybuff[0] = '\0';

    // Verifica se existe o .BIN digitado
    p_mMMSJOS->_strcat(vnomefile,icon_prg[next_pos],".BIN");
    FindFileDir();
    if (vcluster == 0xFFFF) {
      message("Binary File\nNot Found\0", BTCLOSE, 0);
      verro = 1;
    }

    // Verifica se n? ?icone duplicado
    for (vx = 0; vx < ICONSPERLINE * 3; vx++) {
      if (vx != next_pos) {
        if (_strcmp(icon_prg[next_pos],icon_prg[vx])) {
          message("Icon Already Exist\0", BTCLOSE, 0);
          verro = 1;
          break;
        }
      }
    }

    if (verro) {
      icon_pos[next_pos] = 0;
      for (vy = 0; vy < 10; vy++) {
        icon_ico[next_pos][vy] = 0;
        icon_prg[next_pos][vy] = 0;
      }
      return;
    }

    // Mostra e Grava nas Configuracoes o Novo Icone
    if (icon_ico[next_pos][0] != '\0') {
      if (next_pos <= (ICONSPERLINE - 1)) {
        vx = COLINIICONS + (16 + SPACEICONS) * next_pos;
        vy = 10;
      }
      else if (next_pos <= (ICONSPERLINE * 2 - 1)) {
        vx = COLINIICONS + (16 + SPACEICONS) * (next_pos - ICONSPERLINE);
        vy = 28;
      }
      else {
        vx = COLINIICONS + (16 + SPACEICONS) * (next_pos - ICONSPERLINE * 2);
        vy = 46;
      }

      icon_pos[next_pos] = next_pos;

      SaveScreen(56, 24, 16, 16);
      MostraIcone(53, 24, 58);  // Mostra Amplulheta

      p_mMMSJOS->_strcat(vnomefile,"MGI",".CNF");
      vendmsb = 0xE5;
      vendlsb = 0x10;
      GravaArquivoMem(530);

      RestoreScreen(56, 24, 16, 16);

      MostraIcone(vx, vy, next_pos);
      next_pos++;

      message("Icon Created\nSuccessfully\0", BTCLOSE, 0);
    }
  }
*/
}

//-------------------------------------------------------------------------
void CMMSJOS::del_icon(void) {
/*
  byte vx, vy, cc, vpos, vposiconx, vposicony;
  byte mkey, temdelete, dd;
  temdelete = 0;

  // Colocar Icone de Lixeira nos icones do usu?io
  for (cc = 0; cc <= (ICONSPERLINE * 3 - 1); cc++) {
    if (icon_ico[cc][0] != '\0' && icon_prg[cc][0] != '\0') {
      temdelete = 1;

      if (icon_pos[cc] <= (ICONSPERLINE - 1)) {
        vx = COLINIICONS + (16 + SPACEICONS) * icon_pos[cc];
        vy = 10;
      }
      else if (icon_pos[cc] <= (ICONSPERLINE * 2 - 1)) {
        vx = COLINIICONS + (16 + SPACEICONS) * (icon_pos[cc] - ICONSPERLINE);
        vy = 28;
      }
      else {
        vx = COLINIICONS + (16 + SPACEICONS) * (icon_pos[cc] - ICONSPERLINE * 2);
        vy = 46;
      }

      FillRect(vx + 8, vy + 8, 8, 8, White);
      PutImage(icones[11], vx + 8, vy + 8, 8, 8);
    }
  }

  if (temdelete) {
    // Aguardar Touch
    vwaittouch = 1;
    VerifyTouchLcd(WHAITTOUCH);

    // Verificar Posi?o
    vposiconx = COLINIICONS;
    vposicony = 10;
    vpos = 0;

    if (*vposty >= 50) {
      vpos = ICONSPERLINE * 2;
      vposicony = 46;
    }
    else if (*vposty >= 30) {
      vpos = ICONSPERLINE;
      vposicony = 28;
    }

    if (*vpostx >= COLINIICONS && *vpostx <= (COLINIICONS + (16 + SPACEICONS) * ICONSPERLINE) && *vposty >= 10) {
      cc = 1;
      for(vx = (COLINIICONS + (16 + SPACEICONS) * (ICONSPERLINE - 1)); vx >= (COLINIICONS + (16 + SPACEICONS)); vx -= (16 + SPACEICONS)) {
        if (*vpostx >= vx) {
          vpos += ICONSPERLINE - cc;
          vposiconx = vx;
          break;
        }

        cc++;
      }

      if (icon_prg[vpos][0] != '\0' && icon_ico[vpos][0] != '\0') {
        // Confirmar "Dele?o"
        mkey = message("Delete Icon ?\0", BTYES | BTNO, 0);
        if (mkey == BTYES) {
          // Apagar Icone
          icon_pos[vpos] = 0;
          for(cc = 0; cc <= 9; cc++) {
            icon_ico[vpos][cc] = '\0';
            icon_prg[vpos][cc] = '\0';
          }

          // Realocar Demais Icones
          for(cc = vpos + 1; cc <= (ICONSPERLINE * 3 - 1); cc++) {
            icon_pos[cc - 1] = cc - 1;
            for(dd = 0; dd <= 9; dd++) {
              icon_ico[cc - 1][dd] = icon_ico[cc][dd];
              icon_prg[cc - 1][dd] = icon_prg[cc][dd];
            }
          }

          vpos = (ICONSPERLINE * 3 - 1);
          icon_pos[vpos] = 0;
          for(cc = 0; cc <= 9; cc++) {
            icon_ico[vpos][cc] = '\0';
            icon_prg[vpos][cc] = '\0';
          }

          MostraIcone(53, 24, 58);  // Mostra Amplulheta

          p_mMMSJOS->_strcat(vnomefile,"MGI",".CNF");
          vendmsb = 0xE5;
          vendlsb = 0x10;
          GravaArquivoMem(530);
        }
      }
    }

    // Apagar parte inferior ao menu na tela
    FillRect(0, 8, LCDX, LCDY, White);

    // Redesenhar tela
    RedrawMain();
  }
  else
    message("No Icons\nFor Delete\0", BTOK, 0);
*/
}

//-------------------------------------------------------------------------
void CMMSJOS::mgi_setup(void) {
/*
  byte vopc[1], vwb;

  vopc[0] = mgi_flags[0];

  SaveScreen(4,3,121,60);
  showWindow("Configuration\0", 4, 3, 121, 60, BTOK | BTCANCEL);
  GotoXY(6,14);
  writes("Type Comm.:");
  radioset(",USB,SERIAL\0", vopc, 6, 26, WINDISP);

  vwaittouch = 1;
  while (1) {
    vwb = waitButton();

    if (vwb == BTOK || vwb == BTCANCEL)
      break;

    radioset(",USB,SERIAL\0", vopc, 6, 26, WINOPER);
  }

  if (vwb == BTOK) {
    MostraIcone(53, 24, 58);  // Mostra Amplulheta

    mgi_flags[0] = vopc[0];

    p_mMMSJOS->_strcat(vnomefile,"MGI",".CNF");
    vendmsb = 0xE5;
    vendlsb = 0x10;
    GravaArquivoMem(530);
  }

  RestoreScreen(4,3,121,60);
*/
}

