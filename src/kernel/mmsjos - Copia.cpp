/********************************************************************************
*    Programa    : mmsjos.c
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
* |             | 1BFFFFFFh
* +-------------+ 1C000000h
* |             |
* |             |
* |  GPU 64MB   | 
* |             |
* |             |
* +-------------+ 1FFFFFFFh 
*
*--------------------------------------------------------------------------------
*
* Endereços de Perifericos
*
*--------------------------------------------------------------------------------
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
********************************************************************************/

#define __USE_TFT_SCROLL__

#include <circle/usb/usbkeyboard.h>
#include <circlelib/circlelib.h>
#include <circle/machineinfo.h>
#include <circle/debug.h>
#include <circle/util.h>
#include <../lib/circlelib/addon/SDCard/emmc.h>
#include <stddef.h>
#include <stdint.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#include "common/mylib.h"
#include <drivers/ds1307.h>

#define DRIVE       "SD:"
#define FILENAME    "/config.txt"

int __locale_ctype_ptr(int return_value)
{
    (void)(return_value);
    return 0;
}   

CMMSJOS *CMMSJOS::s_pThis = 0;
char CMMSJOS::vkeybuffer[255];

const char* pMonthName[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

CMMSJOS::CMMSJOS (CScrTft *mScrTft, CBTSubSystem *mBluetooth, CTimer *mTimer, CDWHCIDevice *mDWHCI, FATFS *mFileSystem, CEMMCDevice *mEMMC)
: p_mScrTft (mScrTft),  
  #ifdef __USE_CIRCLE_BLUETOOTH__
      p_mBluetooth (mBluetooth),  
  #endif
  p_mTimer (mTimer),  
  p_mDWHCI (mDWHCI), 
  p_mFileSystem (mFileSystem), 
  p_mEMMC (mEMMC)
{
    #ifndef __USE_CIRCLE_BLUETOOTH__
        p_mSerial = new CSerialDevice();
    #endif

    s_pThis = this;
}

CMMSJOS::~CMMSJOS (void)
{
    s_pThis = 0;
    p_mTimer = 0;
    #ifdef __USE_CIRCLE_BLUETOOTH__
        p_mBluetooth = 0;
    #endif
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

    vbufkptr = (unsigned char*)&arrvbufkptr;
    vbuf = (unsigned char*)&arrvbuf;
    mcfgfile = (unsigned char*)&arrmcfgfile;
    vbufptr = vbuf;
    *vbufptr = 0x00;

    vtotmem = (vtotmem >> 20);
    vtotclock = (vtotclock / 1000000);

//    p_mScrTft->locate(0,0, NOREPOS_CURSOR);
//    p_mScrTft->writec(0x08, NOADD_POS_SCR);

    p_mScrTft->ativaCursor();
//    p_mScrTft->clearScr();

    p_mScrTft->printf("OS> MMSJ-OS version %s\n", pVersionSO);
    p_mScrTft->printf("OS> %s at %d%sHz\n", CMachineInfo::Get ()->GetMachineName (), ((vtotclock >= 1000) ? (vtotclock / 1000) : vtotclock), ((vtotclock >= 1000) ? "G" : "M"));
    p_mScrTft->printf("OS> Model %04x\n", CMachineInfo::Get ()->GetMachineModel ());
    p_mScrTft->printf("OS> Revision %04x\n", CMachineInfo::Get ()->GetModelRevision ());
    p_mScrTft->printf("OS> Serial %08x%08x\n", TagSerial.Serial[0], TagSerial.Serial[1]);
    p_mScrTft->printf("OS> MAC %02x.%02x.%02x.%02x.%02x.%02x\n", MACAddress.Address[0], MACAddress.Address[1], MACAddress.Address[2], MACAddress.Address[3], MACAddress.Address[4], MACAddress.Address[5]);
    p_mScrTft->printf("OS> Total Free Memory %dMB\n", vtotmem);
    p_mScrTft->printf("OS> LCD Graphic %dx%d\n", (p_mScrTft->vxgmax + 1), (p_mScrTft->vygmax + 1));
    p_mScrTft->printf("OS> LCD Text %dx%d\n", (p_mScrTft->vxmax + 1), (p_mScrTft->vymax + 1));
    p_mScrTft->printf("OS> Touch Module Found\n");

    if (bcm2835_gpio_lev(41))
        p_mScrTft->printf("OS> Wifi is On\n");

    if (bcm2835_gpio_lev(45))
        p_mScrTft->printf("OS> Bluetooth is On\n");

    pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get ()->GetDevice ("ukbd1", FALSE);

    if (pKeyboard != 0)
    {
        p_mScrTft->printf("OS> USB Keyboard Found\n");
        pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
    }

    pResM = fsMount();
    if (pResM == FR_OK)
        p_mScrTft->printf("OS> Mounting Disk... Done\n");
    else
        p_mScrTft->printf("OS> Mounting Disk... Error (%d)\n",pResM);

    #ifndef __USE_CIRCLE_BLUETOOTH__
        if (p_mSerial->Initialize (9600)) 
        {
            p_mScrTft->printf("OS> Serial Initialized...\n");
        }
    #endif

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

    p_mScrTft->printf("\n");
    putPrompt(noaddline);

    p_mScrTft->ativaCursor();

    // Ativa Teclado Touch
    if (pKeyboard == 0)
        p_mScrTft->funcKey(0,1, 1, 0, 50, 0);

    // Loop principal
    while (1) {
        // Verificar Teclado Touch
        if (pKeyboard == 0)
        {
            vbytepic = p_mScrTft->getKey();
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

                if (p_mScrTft->vcol > p_mScrTft->vxmax) {
                    p_mScrTft->vlin = p_mScrTft->vlin + 1;
                    p_mScrTft->locate(0, p_mScrTft->vlin, NOREPOS_CURSOR);
                }

                p_mScrTft->writec(vbytepic, ADD_POS_SCR);
            }
            else {
                switch (vbytepic) {
                    case 0x0A:  // New Line
                    case 0x0D:  // Enter
                        p_mScrTft->vlin = p_mScrTft->vlin + 1;
                        p_mScrTft->desativaCursor();
                        if (pKeyboard == 0)
                            p_mScrTft->funcKey(0,2, 0, 0, 50, 0);
                        p_mScrTft->locate(0, p_mScrTft->vlin, NOREPOS_CURSOR);
                        *vbufptr = 0x00;
                        processCmd();
                        putPrompt(noaddline);
                        vbufptr = vbuf;
                        *vbufptr = 0x00;
                        if (pKeyboard == 0)
                            p_mScrTft->funcKey(0,1, 1, 0, 50, 0);
                        break;
                    case 0x7F:  // USB Keyboard BackSpace
                    case 0x08:  // Touch Keyboard BackSpace
                        if (p_mScrTft->vcol > vinip) {
                            *vbufptr = '\0';
                            vbufptr--;
                            if (vbufptr < vbuf)
                                vbufptr = vbuf;
                            *vbufptr = '\0';
                            p_mScrTft->vcol = p_mScrTft->vcol - 1;
                            p_mScrTft->locate(p_mScrTft->vcol,p_mScrTft->vlin, NOREPOS_CURSOR);
                            p_mScrTft->writec(0x08, ADD_POS_SCR);
                            p_mScrTft->vcol = p_mScrTft->vcol - 1;
                            p_mScrTft->locate(p_mScrTft->vcol,p_mScrTft->vlin, NOREPOS_CURSOR);
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

//-----------------------------------------------------------------------------
// OS Functions
//-----------------------------------------------------------------------------
void CMMSJOS::processCmd(void) 
{
    char linhacomando[32], linhaarg[32];
    unsigned char *blin = (unsigned char*)vbuf;
    char vlinha[40], vproccd[32];
    unsigned int varg = 0;
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
            p_mScrTft->clearScr();
        }
        else if (!strcmp(linhacomando,"CLEAR") && iy == 5) 
        {
            p_mScrTft->clearScr();
        }
        else if (!strcmp(linhacomando,"VER") && iy == 3) 
        {
            p_mScrTft->printf("MMSJ-OS v%s\n", pVersionSO);
        }
        else if (!strcmp(linhacomando,"LS") && iy == 2) 
        {
            int dirDay, dirMonth /*, dirYear*/ ;
            int dirHour, dirMin;
            DIR Directory;
            FILINFO FileInfo;
            FRESULT Result = f_findfirst (&Directory, &FileInfo, (char*)vdiratu, "*");
            p_mScrTft->printf("\n");

            #if FF_USE_LABEL
                TCHAR* labelDisk;
                DWORD* serieDisk;
                if (f_getlabel (DRIVE, labelDisk, serieDisk) == FR_OK)
                {
                    CString DiskLabel;
                    DiskLabel.Format ("%-17s", labelDisk);
                    p_mScrTft->printf("       Name Disk is %s\n", DiskLabel);
                    p_mScrTft->printf("       Serial Disk is %04x-%04x\n", ((*serieDisk & 0xFFFF0000) >> 16), (*serieDisk & 0x0000FFFF));
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

                    p_mScrTft->printf("%8d %s %02d %02d:%02d %s\n", FileInfo.fsize, pMonthName[dirMonth], dirDay, dirHour, dirMin, (const char *) FileName);
                }

                Result = f_findnext (&Directory, &FileInfo);
            }
            p_mScrTft->printf("\n");
        }
        else 
        {
            if (!strcmp(linhacomando,"RM") && iy == 2) 
            {
                pResF = f_unlink((char*)vparam);
                if (pResF != FR_OK)
                {
                    p_mScrTft->printf("Delete File/Folder Error (%d)\n", pResF);
                }
                else
                    p_mScrTft->printf("\n");
            }
            else if (!strcmp(linhacomando,"REN") && iy == 3) 
            {
                pResF = f_rename(retPathAndFile((char*)vparam), (char*)vparam2);

                if (pResF != FR_OK)
                {
                    p_mScrTft->printf("Rename Error (%d)\n", pResF);
                }
                else
                    p_mScrTft->printf("\n");
            }
            else if (!strcmp(linhacomando,"CP") && iy == 2) 
            {
                /* ver como pegar */
            }
            else if (!strcmp(linhacomando,"MD") && iy == 2) 
            {
                pResF = f_mkdir((char*)vparam);
                if (pResF != FR_OK)
                {
                    p_mScrTft->printf("Create Folder Error (%d)\n", pResF);
                }
                else
                    p_mScrTft->printf("\n");
            }
            else if (!strcmp(linhacomando,"CD") && iy == 2) 
            {            
                if (!strcmp((char*)vparam,".") && strlen((char*)vparam) == 1)
                    p_mScrTft->printf("%s\n",vdiratu);
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
                                    p_mScrTft->printf("Path Not Found (%d)\n", pResF);
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
                                        p_mScrTft->printf("Path Not Found (%d)\n", pResF);
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
                p_mScrTft->printf("\n");
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
                        p_mScrTft->printf("  Write RTC Error (%d)\n",pData.Error);

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

                    p_mScrTft->printf("  Date is (dd/mm/yyyy) %s/%s/20%s\n", strDay, strMon, strYear);
                    p_mScrTft->printf("  Time is (hh:mm:ss) %s:%s:%s\n", strHrs, strMin, strSec);
                }
                else
                    p_mScrTft->printf("  Read RTC Error (%d)\n",pData.Error);
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
            #ifdef __USE_SERIAL_BLUETOOTH__
                else if (!strcmp(linhacomando,"LOADSBT") && iy == 7) 
                {
                    load232();
                    ix = 255;
                }
                else if (!strcmp(linhacomando,"SENDSBT") && iy == 7) 
                {
                    // A definir
                    ix = 255;
                }
            #endif
            #ifdef __USE_CIRCLE_BLUETOOTH__
                else if (!strcmp(linhacomando,"LOADBT") && iy == 7) 
                {
                    loadbt();
                    ix = 255;
                }
                else if (!strcmp(linhacomando,"SENDBT") && iy == 7) 
                {
                    // A definir
                    ix = 255;
                }
                else if (!strcmp(linhacomando,"CONNBT") && iy == 7) 
                {
                    load232();
                    ix = 255;
                }
                else if (!strcmp(linhacomando,"FINDBT") && iy == 6) 
                {
                    p_mScrTft->printf("Searching Bluetooth Devices...\n");

                    p_mScrTft->printf("Inquiry is running for %u seconds\n", INQUIRY_SECONDS);

                    CBTInquiryResults *pInquiryResults = p_mBluetooth->Inquiry (INQUIRY_SECONDS);
                    if (pInquiryResults == 0)
                    {
                        p_mScrTft->printf("Inquiry failed\n");
                    }

                    p_mScrTft->printf("Inquiry complete, %u device(s) found", pInquiryResults->GetCount ());

                    if (pInquiryResults->GetCount () > 0)
                    {
                        p_mScrTft->printf("# BD address        Class  Name");

                        for (unsigned nDevice = 0; nDevice < pInquiryResults->GetCount (); nDevice++)
                        {
                            const u8 *pBDAddress = pInquiryResults->GetBDAddress (nDevice);
                            assert (pBDAddress != 0);
                            
                            const u8 *pClassOfDevice = pInquiryResults->GetClassOfDevice (nDevice);
                            assert (pClassOfDevice != 0);
                            
                            p_mScrTft->printf("%u %02X:%02X:%02X:%02X:%02X:%02X %02X%02X%02X %s\n",
                                    nDevice+1,
                                    (unsigned) pBDAddress[5],
                                    (unsigned) pBDAddress[4],
                                    (unsigned) pBDAddress[3],
                                    (unsigned) pBDAddress[2],
                                    (unsigned) pBDAddress[1],
                                    (unsigned) pBDAddress[0],
                                    (unsigned) pClassOfDevice[2],
                                    (unsigned) pClassOfDevice[1],
                                    (unsigned) pClassOfDevice[0],
                                    pInquiryResults->GetRemoteName (nDevice));
                        }
                    }

                    delete pInquiryResults;

                    ix = 255;
                }
            #endif
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
                        p_mScrTft->printf("Loading File Error...\n");
                    }

                    ix = 255;
                }
                else {
                    // Se nao tiver, mostra erro
                    p_mScrTft->printf("Invalid Command or File Name\n");
                    ix = 255;
                }*/
            }

            if (ix != 255)  
            {
                if (((vpicret) && (vbytepic != RETURN_OK)) || ((!vpicret) && (vretfat != RETURN_OK))) 
                {
                    p_mScrTft->printf("Command unsuccessfully\n\0");
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
                                    p_mScrTft->printf("    IP Addr: %s\n", vlinha);
                                    break;
                                case 2:
                                    p_mScrTft->printf("    SubMask: %s\n", vlinha);
                                    break;
                                case 3:
                                    p_mScrTft->printf("    Gateway: %s\n", vlinha);
                                    break;
                                case 4:
                                    p_mScrTft->printf("   DNS Prim: %s\n", vlinha);
                                    break;
                                case 5:
                                    p_mScrTft->printf("    DNS Sec: %s\n", vlinha);
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
        p_mScrTft->vlin = p_mScrTft->vlin + 1;

    p_mScrTft->locate(0,p_mScrTft->vlin, NOREPOS_CURSOR);

    p_mScrTft->printf("#%s>",vdiratu);

    vinip = p_mScrTft->vcol;

    p_mScrTft->ativaCursor();
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
        p_mScrTft->printf("File Not Found (%d)\n", Result);
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
                    p_mScrTft->vcorf = vval;
                }
                else if (!strcmp((char*)vset,"BCOLOR") && vigual == 6) {
                    vval = atoi((char*)vparam);
                    p_mScrTft->vcorb = vval;
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
                    p_mScrTft->printf("Config file syntax error...\n");
                }

                break;
            }

            mcfgfileptr++;
        }

        if (ipos <= 40 && ptipo) {
            p_mScrTft->printf("Settings applied successfully...\n");
        }

        p_mScrTft->writec(0x08, NOADD_POS_SCR);
        p_mScrTft->ativaCursor();
    }
    else {
        if (ptipo) {
            p_mScrTft->printf("Loading config file error...\n");
        }
        vret = 0;
    }

    return vret;
}

//-----------------------------------------------------------------------------
void CMMSJOS::load232(void)
{
    unsigned char pByteComm = 0x00, lResult, vOldLin;
    unsigned int pCount, ix;
    unsigned long vretfat;
    unsigned char pBuffer[255];
    unsigned char pFileName[15];
    unsigned char *pMemLoad = (unsigned char*)vMemSystemArea;

    p_mScrTft->printf("Setting Serial BT Mode...\n");

    pBuffer[0] = 'A';
    pBuffer[1] = 'T';
    pBuffer[2] = '+';
    pBuffer[3] = 'M';
    pBuffer[4] = 'O';
    pBuffer[5] = 'D';
    pBuffer[6] = 'E';
    pBuffer[7] = '0';
    p_mSerial->Write(pBuffer,8);

    p_mScrTft->printf("Setting Serial BT Name...");

    pBuffer[0] = 'A';
    pBuffer[1] = 'T';
    pBuffer[2] = '+';
    pBuffer[3] = 'N';
    pBuffer[4] = 'A';
    pBuffer[5] = 'M';
    pBuffer[6] = 'E';
    pBuffer[7] = 'M';
    pBuffer[8] = 'M';
    pBuffer[9] = 'S';
    pBuffer[10] = 'J';
    pBuffer[11] = 'B';
    pBuffer[12] = 'T';
    p_mSerial->Write(pBuffer,13);

    p_mScrTft->printf("Verifying...");

    p_mSerial->Read(&pBuffer, 17);

    p_mScrTft->printf("Done\n");

    p_mScrTft->printf("Establishing Communication...\n");

    pBuffer[0] = 0x00;
    while (pBuffer[0] != 0xDD)
        p_mSerial->Read(&pBuffer, 2);

    if (pBuffer[1] == 0xDD)
    {
        p_mScrTft->printf("Communication Established...\n");

        pBuffer[0] = 0xDD;
        pBuffer[1] = 0xDD;
        p_mSerial->Write(pBuffer,2);

        p_mScrTft->printf("Receiving File Name...\n");
        pCount = 0;
        lResult = 1;

        while(1)
        {
            pBuffer[0] = 0x00;
            while (pBuffer[0] != 0xDD)
                p_mSerial->Read(&pBuffer, 2);

            pByteComm = pBuffer[1];

            pBuffer[0] = 0xDD;
            pBuffer[1] = pByteComm;
            p_mSerial->Write(pBuffer,2);

            pBuffer[0] = 0x00;
            while (pBuffer[0] != 0xEE)
                p_mSerial->Read(&pBuffer, 2);

            if (!pBuffer[1])
            {
                pFileName[pCount] = toupper(pByteComm);
                pCount++;
            }

            if (pCount == 15)
                break;
        }

        if (lResult)
        {
            p_mScrTft->printf("Receiving File %s\n",pFileName);

            vOldLin = p_mScrTft->vlin;
            pCount = 0;

            while(1)
            {
                if (pCount % 100 == 0) 
                {
                    p_mScrTft->locate(0, vOldLin, NOREPOS_CURSOR);
                    p_mScrTft->printf("Receiving... %d Byte(s)",pCount);
                }

                pBuffer[0] = 0x00;
                while (pBuffer[0] != 0xDD && pBuffer[0] != 0xEE)
                    p_mSerial->Read(&pBuffer, 1);

                if (pBuffer[0] == 0xDD)
                {
                    p_mSerial->Read(&pBuffer, 1);
                    pByteComm = pBuffer[0];

                    pBuffer[0] = 0xDD;
                    pBuffer[1] = pByteComm;
                    p_mSerial->Write(pBuffer,2);

                    pBuffer[0] = 0x00;
                    while (pBuffer[0] != 0xEE)
                        p_mSerial->Read(&pBuffer, 1);

                    p_mSerial->Read(&pBuffer, 1);

                    if (!pBuffer[0])
                    {
                        *pMemLoad++ = pByteComm;
                        pCount++;
                    }
                }
                else if (pBuffer[0] == 0xEE)
                {
                    p_mSerial->Read(&pBuffer, 1);

                    if (pBuffer[0] == 0xDD)
                    {
                        // Salvar Arquivo
                        p_mScrTft->locate(0, vOldLin, NOREPOS_CURSOR);
                        p_mScrTft->printf("Received... %d Byte(s)     ",pCount);
                        p_mScrTft->printf("\nSaving File...\n");

//                        vretfat = fsOpenFile((char*)pFileName);
                        if (vretfat != RETURN_OK) {
  //                          vretfat = fsCreateFile((char*)pFileName);
                        }

                        if (vretfat == RETURN_OK)
                        {
                            pMemLoad = (unsigned char*)vMemSystemArea;
                            ix = 0;
                            while(1)
                            {
//                                vretfat = fsWriteFile((char*)pFileName, ix, pMemLoad, 128);
                                if (vretfat != RETURN_OK)
                                    break;

                                if (ix >= pCount)
                                    break;

                                pMemLoad += 128;
                                ix += 128;
                            }

  //                          fsCloseFile((char*)pFileName, 1);

                            if (vretfat == RETURN_OK)
                                p_mScrTft->printf("File Saved...\n");
                        }

                        if (vretfat != RETURN_OK)
                            p_mScrTft->printf("Erro Saving File (%d)\n",vretfat);

                        break;
                    }
                }
            }
        }
    }
    else 
        p_mScrTft->printf("Wrong 2nd.Start (0x%02X) Received.\n", pByteComm);
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
                p_mScrTft->locate(0, p_mScrTft->vlin, NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x0A) 
            {
                p_mScrTft->vlin = p_mScrTft->vlin + 1;
                p_mScrTft->locate(p_mScrTft->vcol, p_mScrTft->vlin, NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x1A || *mcfgfileptr == 0x00) 
            {
                break;
            }
            else 
            {
                if (*mcfgfileptr >= 0x20 && *mcfgfileptr <= 0x7F)
                    p_mScrTft->writec(*mcfgfileptr, ADD_POS_SCR);
                else
                    p_mScrTft->writec(0x20, ADD_POS_SCR);
            }

            mcfgfileptr++;
            vsizefile--;
        }
    }
    else 
    {
        p_mScrTft->printf("Loading file error...\n");
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
