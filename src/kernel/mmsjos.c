/********************************************************************************
*    Programa    : mmsjos.c
*    Objetivo    : SO do modulo RASPBERRY PI ZERO W
*    Criado em   : 02/11/2018
*    Programador : Moacir Jr.
*--------------------------------------------------------------------------------
* Data        Versão  Responsavel  Motivo
* 02/11/2018  0.1     Moacir Jr.   Criação Versão Beta
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
#define __USE_TFT_VDG__
//#define __USE_UART_MON__
#define __USE_FAT32_SDDISK__

#include <stddef.h>
#include <stdint.h>
#include <drivers/uart.h>
#include <kernel/mem.h>
#include <kernel/atag.h>
#include <kernel/mmsjos.h>
#include <kernel/mmsjos_api.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#ifdef __USE_TFT_VDG__
    #include <drivers/lcd_tch.h>
    #include <drivers/lcd_vdg_api.h>
#endif
#include <drivers/spi_manual.h> 
#include <kernel/interrupts.h>
#include <disk/diskio.h>
#include "common/mylib.h"
#include "drivers/rpi-usb-api.h"            // This units header

unsigned char* pVersionSO = "0.1";

unsigned int vcorwb;
unsigned int vcorwf;
unsigned char* vMemSystemArea = (unsigned char*)0x00700000;       // 1MB - Atualizar sempre que a compilacao passar desse valor
unsigned char* vMemUserArea = (unsigned char*)0x00800000;   // 440MB - Atualizar sempre que a compilacao passar desse valor
unsigned char paramVDG[255];
unsigned char arrgDataBuffer[520];
unsigned char arrvdiratu[128];
unsigned char arrvdiratup[128];
unsigned char arrvbufkptr[128];
unsigned char arrvbuf[64];
unsigned char arrmcfgfile[12288];
unsigned long vtotclock;
uint16_t vboardmodel;
uint16_t vboardrev;
uint8_t vboardmacaddr[8];
uint16_t vboardserial;

mBoxInfoResp MBOX_INFO_BOARD;
FAT32_DIR varToPtrvdir;
FAT32_DIR *vdir = &varToPtrvdir;
DISK varToPtrvdisk;
DISK *vdisk = &varToPtrvdisk;

unsigned char strValidChars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^&'@{}[],$=!-#()%.+~_";

unsigned char vmesc[12][3] = {{'J','a','n'},{'F','e','b'},{'M','a','r'},
                           {'A','p','r'},{'M','a','y'},{'J','u','n'},
                           {'J','u','l'},{'A','u','g'},{'S','e','p'},
                           {'O','c','t'},{'N','o','v'},{'D','e','c'}};

int __locale_ctype_ptr(int return_value)
{
    return 0;
}   

//-----------------------------------------------------------------------------
// Principal
//-----------------------------------------------------------------------------
void mmsjos_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    // Declare as unused
    (void) r0;
    (void) r1;
    (void) atags;

    bcm2835_init();

    vtotmem = mem_init( /*(atag_t *)atags*/ );

    vtotclock = get_clock_hz(MBOX_GET_CLOCK_RATE);
 /*   MBOX_INFO_BOARD = get_info_arm(MBOX_GET_BOARD_MODEL);
    vboardmodel = MBOX_INFO_BOARD.modelrev;
    MBOX_INFO_BOARD = get_info_arm(MBOX_GET_BOARD_REVISION);
    vboardrev = MBOX_INFO_BOARD.modelrev;
    MBOX_INFO_BOARD = get_info_arm(MBOX_GET_BOARD_MAC_ADDRESS);
    vboardmacaddr[0] = MBOX_INFO_BOARD.byte00;
    vboardmacaddr[1] = MBOX_INFO_BOARD.byte01;
    vboardmacaddr[2] = MBOX_INFO_BOARD.byte02;
    vboardmacaddr[3] = MBOX_INFO_BOARD.byte03;
    vboardmacaddr[4] = MBOX_INFO_BOARD.byte04;
    vboardmacaddr[5] = MBOX_INFO_BOARD.byte05;
    vboardmacaddr[6] = MBOX_INFO_BOARD.byte06;
    vboardmacaddr[7] = MBOX_INFO_BOARD.byte07;
    MBOX_INFO_BOARD = get_info_arm(MBOX_GET_BOARD_SERIAL);
    vboardserial = MBOX_INFO_BOARD.serial;*/

    spi_man_init();

    #ifdef __USE_TFT_VDG__
        TFT_Init();

        Touch_Init();
    #endif

    #ifdef __USE_UART_MON__
        uart_init();
    #endif

    unsigned int vbytepic = 0, vbytevdg;
    unsigned int ix = 0, vvcol = 0;
    unsigned char *vbufptr;
    char *sqtdtam;
    unsigned char vret;

    gDataBuffer = &arrgDataBuffer;
    vdiratu = &arrvdiratu;
    vdiratup = &arrvdiratup;
    vbufkptr = &arrvbufkptr;
    vbuf = &arrvbuf;
    mcfgfile = &arrmcfgfile;
    vbufptr = vbuf;
    *vbufptr = 0x00;

    voutput = 0x01; // Indica Padrão Inicial Video TEXTO = 1, MGI = 2
    voverx = 0;   // Indica sem overlay, usando tela de texto.
    vovery = 0;   // Indica sem overlay, usando tela de texto.
    vxmaxold = 0;
    vymaxold = 0;
    vtotmem = (vtotmem >> 20);
    vtotclock = (vtotclock / 1000000);

    // Recuperar informacoes do Video
    #ifdef __USE_TFT_VDG__
        paramVDG[0] = 0x01;
        paramVDG[1] = 0xEF;
        commVDG(paramVDG);

        vbytevdg = paramVDG[0];
        vxgmax = paramVDG[1] << 8;
        vxgmax = vxgmax | paramVDG[2];
        vygmax = paramVDG[3] << 8;
        vygmax = vygmax | paramVDG[4];

        if (vxgmax != 319 || vygmax != 239) {
            vxgmax = 319;
            vygmax = 239;
        }

        vxmax = ((vxgmax + 1) / 8) - 1;
        vymax = ((vygmax + 1) / 10) - 1;
        vlin = 0;
        vcol = 0;
        inten = 0;
        vcorf = White;
        vcorb = Black;

        locate(0,0, NOREPOS_CURSOR);
        writec(0x08, vcorf, vcorb, NOADD_POS_SCR);
        ativaCursor();
        clearScr(vcorb);
    #endif

    printf("Running MMSJ-OS version %s\n", pVersionSO);
    printf(" Raspberry PI Zero W v1.1 at %d%sHz.\n", ((vtotclock >= 1000) ? (vtotclock / 1000) : vtotclock), ((vtotclock >= 1000) ? "G" : "M"));
/*    printf("    Board Model %04x\n", vboardmodel);
    printf("    Board Revision %04x\n", vboardrev);
    printf("    Board MAC %02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n", vboardmacaddr[0], vboardmacaddr[1], vboardmacaddr[2], vboardmacaddr[3], vboardmacaddr[4], vboardmacaddr[5], vboardmacaddr[6], vboardmacaddr[7]);
    printf("    Board Serial %08x\n", vboardserial);*/
    printf(" Total Free Memory %dMB\n", vtotmem);

    #ifdef __USE_TFT_VDG__
        printf("VDG Configurations...\n");
        printf(" Graphic %dx%d, Text %dx%d...\n", (vxgmax + 1), (vygmax + 1), (vxmax + 1), (vymax + 1));

        if ((vbytevdg & 0x0002) == 0x02)
            printf(" Touch Module... Found.\n");
        else
            printf(" Touch Module... NOT Found.\n");
    #endif

    printf("Initializing FileSystem... ");
    vbytepic = fsMountDisk();

    interrupts_init();
    TFT_EN_INT();
    USB_EN_INIT();

    if (vbytepic != RETURN_OK) 
    {
        if (vbytepic == ERRO_B_OPEN_DISK)
            printf("\nError Mounting Disk. Can't Initialize (%d).\n", vbytepic);
        else
            printf("\nError Mounting Disk. Can't Read (%d).\n", vbytepic);
    } 
    else
        printf("Done.\n");

    /* Initialize USB system we will want keyboard and mouse */
    UsbInitialise();

    /* Display the USB tree */
    UsbShowTree(UsbGetRootHub(), 1, '+');
    printf("\n");

    /* Detect the first keyboard on USB bus */
    uint8_t firstKbd = 0;

    // Inicio SO
    *vdiratup++ = '/';
    *vdiratup = '\0';

    printf("\n");
    putPrompt(noaddline);

    #ifdef __USE_TFT_VDG__
        ativaCursor();

        // Ativa Teclado Touch
        funcKey(0,1, 1, 0, 50, 0);
    #endif

    // Loop principal
    while (1) {
        UsbCheckForChange();

        if (!firstKbd)
        {
            for (int i = 1; i <= MaximumDevices; i++) 
            {
                if (IsKeyboard(i)) 
                {
                    firstKbd = i;
                    break;
                }
            }
            
            if (firstKbd) 
            {
                printf("Keyboard detected\n");
                putPrompt(noaddline);
            }
        }

        if (firstKbd) 
        {
            RESULT status;
            uint8_t buf[8];
            status = HIDReadReport(firstKbd, 0, (uint16_t)USB_HID_REPORT_TYPE_INPUT << 8 | 0, &buf[0], 8);
            if (status == OK)
            {
                printf("HID KBD REPORT: Byte1: 0x%02x Byte2: 0x%02x, Byte3: 0x%02x, Byte4: 0x%02x\n",
                    buf[0], buf[1], buf[2], buf[3]);
                printf("                Byte5: 0x%02x Byte6: 0x%02x, Byte7: 0x%02x, Byte8: 0x%02x\n",
                    buf[4], buf[5], buf[6], buf[7]);
            }
            else printf("Status error: %08x\n", status);
            putPrompt(noaddline);
        }

        #ifdef __USE_TFT_VDG__
            blinkCursor();

            // Verificar Teclado Touch
            getKey();

            vbytepic = paramVDG[0];

            // Verifica Retorno
            if (vbytepic != 0) {
                if (vbytepic >= 0x20 && vbytepic <= 0x7F) {
                    *vbufptr = vbytepic;
                    vbufptr++;

                    if (vbufptr > vbuf + 31)
                        vbufptr = vbuf + 31;

                    if (vcol > vxmax) {
                        vlin = vlin + 1;
                        locate(0, vlin, NOREPOS_CURSOR);
                    }

                    writec(vbytepic, vcorf, vcorb, ADD_POS_SCR);
                }
                else {
                    switch (vbytepic) {
                        case 0x0D:  // Enter
                            vlin = vlin + 1;
                            funcKey(0,2, 0, 0, 50, 0);
                            locate(0, vlin, NOREPOS_CURSOR);
                            *vbufptr = 0x00;
                            processCmd();
                            putPrompt(noaddline);
                            vbufptr = vbuf;
                            *vbufptr = 0x00;
                            funcKey(0,1, 1, 0, 50, 0);
                            break;
                        case 0x08:  // BackSpace
                            if (vcol > vinip) {
                                *vbufptr = '\0';
                                vbufptr--;
                                if (vbufptr < vbuf)
                                    vbufptr = vbuf;
                                *vbufptr = '\0';
                                vcol = vcol - 1;
                                locate(vcol,vlin, NOREPOS_CURSOR);
                                writec(0x08, vcorf, vcorb, ADD_POS_SCR);
                                vcol = vcol - 1;
                                locate(vcol,vlin, NOREPOS_CURSOR);
                            }

                            break;
                        case 0x09:  // TAB
                            // Procurar proximo nome de arquivo comecando pelo que
                            // foi digitado ate o momento no buffer
                            break;
                    }
                }
            }
        #endif

        #ifdef __USE_UART_MON__
            putPrompt(noaddline);

            gets(vbuf,256);
            puts(vbuf);
            putc('\n');

            processCmd();
        #endif
    }
}

//-----------------------------------------------------------------------------
// Comm Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// OS Functions
//-----------------------------------------------------------------------------
void funcKey(unsigned char vambiente, unsigned char vshow, unsigned char venter, unsigned char vtipo, unsigned int x, unsigned int y) {
    unsigned int vkeyx, vkeyy;

    // Calcula posicao do teclado
    #ifdef __USE_TFT_VDG__
        if (!vambiente) {
            vkeyx = x;

            if (vlin < (vymax - 10))
                vkeyy = (vlin + 1)  * 10;
            else
                vkeyy = (vlin - 11) * 10;
        }
        else {
            vkeyx = x;

            if (y < (vygmax - 100))
                vkeyy = y + 1;
            else
                vkeyy = y - 100;
        }

        paramVDG[0] = 9;
        paramVDG[1] = 0xDB;
        paramVDG[2] = vkeyx >> 8;
        paramVDG[3] = vkeyx;
        paramVDG[4] = vkeyy >> 8;
        paramVDG[5] = vkeyy;
        paramVDG[6] = vshow;  // 1 - Ativa o Teclado com show on touch, 2 - Esconde e desativa
        paramVDG[7] = venter; // Tipo de tecla final (0 - end ou 1 - sinal de enter)
        paramVDG[8] = 0x00;   // Caps (0 - Minus, 1 - Maius)
        paramVDG[9] = vtipo;  // Tipo Teclado (0 - alpha, 1 - numerico, 2 - simbolos)

        commVDG(paramVDG);
    #endif
}

//-----------------------------------------------------------------------------
void processCmd(void) {
    char linhacomando[32], linhaarg[32], vloop;
    unsigned char *blin = (unsigned char*)vbuf, vbuffer[128];
    char vlinha[40];
    unsigned int varg = 0, vcrc, vcrcpic;
    unsigned int ix, iy, iz = 0, ikk = 0;
    unsigned int vbytepic = 0, vrecfim;
    unsigned char *vdirptr = (unsigned char*)&vdir;
    unsigned char cuntam, vparam[32], vparam2[16], vpicret;
    char *sqtdtam;
    unsigned long vretfat;
    long vqtdtam;

    // Separar linha entre comando e argumento
    linhacomando[0] = '\0';
    linhaarg[0] = '\0';
    ix = 0;
    iy = 0;
    while (*blin != 0) {
        if (!varg && *blin == 0x20) {
            varg = 0x01;
            linhacomando[ix] = '\0';
            iy = ix;
            ix = 0;
        }
        else {
            if (!varg)
                linhacomando[ix] = toupper(*blin);
            else
                linhaarg[ix] = toupper(*blin);
            ix++;
        }

        blin++;
    }

    if (!varg) {
        linhacomando[ix] = '\0';
        iy = ix;
    }
    else {
        linhaarg[ix] = '\0';

        ikk = 0;
        iz = 0;
        varg = 0;
        while (ikk < ix) {
            if (linhaarg[ikk] == 0x20)
                varg = 1;
            else {
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
    if (linhacomando[0] != 0) {
        if (!strcmp(linhacomando,"CLS") && iy == 3) {
            clearScr(vcorb);
        }
        else if (!strcmp(linhacomando,"CLEAR") && iy == 5) {
            clearScr(vcorb);
        }
        else if(!strcmp(linhacomando,"MGI") && iy == 3) {
            if (voutput == 1)
                startMGI();
        }
        else if (!strcmp(linhacomando,"VER") && iy == 3) {
            printf("MMSJ-OS v%s\n", pVersionSO);
        }
        else if (!strcmp(linhacomando,"LS") && iy == 2) {
//            clearScr(vcorb);
//            extDebugActiv = 1;
            vretfat = fsFindInDir(NULL, TYPE_FIRST_ENTRY);
//            extDebugActiv = 0;

            if (vretfat >= ERRO_D_START) {
                printf("File not found (%d)\n",vretfat);
            }
            else {
                printf("\n");
                while (1) {
					if (vdir->Attr != ATTR_VOLUME && vdir->Attr != ATTR_LONG_NAME) {
    					memset(vbuffer, 0x0, 128);
                        vdirptr = (unsigned char*)&vdir;

                        for(ix = 40; ix <= 79; ix++)
                            vbuffer[ix] = *vdirptr++;

                        if (vdir->Attr != ATTR_DIRECTORY) {
                            // Reduz o tamanho a unidade (GB, MB ou KB)
                            vqtdtam = vdir->Size;

                            if ((vqtdtam & 0xC0000000) != 0) {
                                cuntam = 'G';
                                vqtdtam = ((vqtdtam & 0xC0000000) >> 30) + 1;
                            }
                            else if ((vqtdtam & 0x3FF00000) != 0) {
                                cuntam = 'M';
                                vqtdtam = ((vqtdtam & 0x3FF00000) >> 20) + 1;
                            }
                            else if ((vqtdtam & 0x000FFC00) != 0) {
                                cuntam = 'K';
                                vqtdtam = ((vqtdtam & 0x000FFC00) >> 10) + 1;
                            }
                            else
                                cuntam = ' ';

                            // Transforma para decimal
        					memset(sqtdtam, 0x0, 10);
                            itoa(vqtdtam, sqtdtam, 10);

                            // Primeira Parte da Linha do dir, tamanho
                            for(ix = 0; ix <= 3; ix++) {
                                if (sqtdtam[ix] == 0)
                                    break;
                            }

                            iy = (4 - ix);

                            for(ix = 0; ix <= 3; ix++) {
                                if (iy <= ix) {
                                    ikk = ix - iy;
                                    vbuffer[ix] = sqtdtam[ix - iy];
                                }
                                else
                                    vbuffer[ix] = ' ';
                            }

                            vbuffer[4] = cuntam;
                        }
                        else {
                            vbuffer[0] = ' ';
                            vbuffer[1] = ' ';
                            vbuffer[2] = ' ';
                            vbuffer[3] = ' ';
                            vbuffer[4] = '0';
                        }

                        vbuffer[5] = ' ';

                        // Segunda parte da linha do dir, data ult modif
                        // Mes
                        vqtdtam = (vdir->UpdateDate & 0x01E0) >> 5;
                        if (vqtdtam < 1 || vqtdtam > 12)
                            vqtdtam = 1;

                        vqtdtam--;

                        vbuffer[6] = vmesc[vqtdtam][0];
                        vbuffer[7] = vmesc[vqtdtam][1];
                        vbuffer[8] = vmesc[vqtdtam][2];
                        vbuffer[9] = ' ';

                        // Dia
                        vqtdtam = vdir->UpdateDate & 0x001F;
      					memset(sqtdtam, 0x0, 10);
                        itoa(vqtdtam, sqtdtam, 10);

                        if (vqtdtam < 10) {
                            vbuffer[10] = '0';
                            vbuffer[11] = sqtdtam[0];
                        }
                        else {
                            vbuffer[10] = sqtdtam[0];
                            vbuffer[11] = sqtdtam[1];
                        }
                        vbuffer[12] = ' ';

                        // Ano
                        vqtdtam = ((vdir->UpdateDate & 0xFE00) >> 9) + 1980;
      					memset(sqtdtam, 0x0, 10);
                        itoa(vqtdtam, sqtdtam, 10);

                        vbuffer[13] = sqtdtam[0];
                        vbuffer[14] = sqtdtam[1];
                        vbuffer[15] = sqtdtam[2];
                        vbuffer[16] = sqtdtam[3];
                        vbuffer[17] = ' ';

                        // Terceira parte da linha do dir, nome.ext
                        ix = 18;
                        varg = 0;
                        while (vdir->Name[varg] != 0x20 && vdir->Name[varg] != 0x00 && varg <= 7) {
                            vbuffer[ix] = vdir->Name[varg];
                            ix++;
                            varg++;
                        }

                        vbuffer[ix] = '.';
                        ix++;

                        varg = 0;
                        while (vdir->Ext[varg] != 0x20 && vdir->Ext[varg] != 0x00 && varg <= 2) {
                            vbuffer[ix] = vdir->Ext[varg];
                            ix++;
                            varg++;
                        }

                        if (varg == 0) {
                            ix--;
                            vbuffer[ix] = ' ';
                            ix++;
                        }

                        // Quarta parte da linha do dir, "/" para diretorio
                        if (vdir->Attr == ATTR_DIRECTORY) {
                            ix--;
                            vbuffer[ix] = '/';
                            ix++;
                        }

                        vbuffer[ix] = '\0';

                        for(ix = 0; ix <= 39; ix++)
                            vlinha[ix] = vbuffer[ix];
					}
					else if (vdir->Attr == ATTR_VOLUME) {
      					memset(vlinha, 0x20, 40);
					    vlinha[5]  = 'D';
					    vlinha[6]  = 'i';
					    vlinha[7]  = 's';
					    vlinha[8]  = 'k';
					    vlinha[9]  = ' ';
					    vlinha[10] = 'N';
					    vlinha[11] = 'a';
					    vlinha[12] = 'm';
					    vlinha[13] = 'e';
					    vlinha[14] = ' ';
					    vlinha[15] = 'i';
					    vlinha[16] = 's';
					    vlinha[17] = ' ';
					    ix = 18;
					    varg = 0;
                        while (vdir->Name[varg] != 0x00 && varg <= 7) {
                            vlinha[ix] = vdir->Name[varg];
                            ix++;
                            varg++;
                        }

                        varg = 0;
                        while (vdir->Ext[varg] != 0x00 && varg <= 2) {
                            vlinha[ix] = vdir->Ext[varg];
                            ix++;
                            varg++;
                        }

                        vlinha[ix] = '\0';
					}
                    else {
                        vlinha[0] = '\0';
                    }

                    // Mostra linha
                    if (vlinha[0])
                    {
                        printf("%s\n",vlinha);
                    }

                    // Verifica se Tem mais arquivos no diretorio
					for (ix = 0; ix <= 7; ix++) {
					    vparam[ix] = vdir->Name[ix];
						if (vparam[ix] == 0x20) {
							vparam[ix] = '\0';
							break;
					    }
					}

					vparam[ix] = '\0';

					if (vdir->Name[0] != '.') {
					    vparam[ix] = '.';
					    ix++;
    					for (iy = 0; iy <= 2; iy++) {
    					    vparam[ix] = vdir->Ext[iy];
    						if (vparam[ix] == 0x20) {
    							vparam[ix] = '\0';
    							break;
    					    }
    					    ix++;
    					}
						vparam[ix] = '\0';
					}

    				if (fsFindInDir((char*)vparam, TYPE_NEXT_ENTRY) >= ERRO_D_START) {
                        printf("\n", vcorf, vcorb);
    					break;
    				}
                }
            }
        }
        else {
            if (!strcmp(linhacomando,"RM") && iy == 2) {
                vretfat = fsDelFile(linhaarg);
            }
            else if (!strcmp(linhacomando,"REN") && iy == 3) {
                vretfat = fsRenameFile((char*)vparam, (char*)vparam2);
            }
            else if (!strcmp(linhacomando,"CP") && iy == 2) {
      			ikk = 0;

          		if (fsOpenFile((char*)vparam) != RETURN_OK) {
                    vretfat = ERRO_B_NOT_FOUND;
                }
                else {
              		if (fsOpenFile((char*)vparam2) != RETURN_OK) {
            			if (fsCreateFile((char*)vparam2) != RETURN_OK) {
                              vretfat = ERRO_B_CREATE_FILE;
                        }
                    }
                }

                while (vretfat == RETURN_OK) {
          			if (fsReadFile((char*)vparam, ikk, vbuffer, 128) > 0) {
        				if (fsWriteFile((char*)vparam2, ikk, vbuffer, 128) != RETURN_OK) {
                            vretfat = ERRO_B_WRITE_FILE;
                            break;
                        }

                        ikk += 128;
                    }
                    else
                        break;
                }
            }
            else if (!strcmp(linhacomando,"MD") && iy == 2) {
                vretfat = fsMakeDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"CD") && iy == 2) {
                vretfat = fsChangeDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"RD") && iy == 2) {
                vretfat = fsRemoveDir(linhaarg);
            }
            else if (!strcmp(linhacomando,"DATE") && iy == 4) {
                vpicret = 1;
                /* ver como pegar */
            }
            else if (!strcmp(linhacomando,"TIME") && iy == 4) {
                vpicret = 1;
                /* ver como pegar */
            }
            else if (!strcmp(linhacomando,"IFCONFIG") && iy == 8) {
                vpicret = 1;
                /* ver como pegar */
            }
/*            else if (!strcmp(linhacomando,"FORMAT") && iy == 6) {
                vretfat = fsFormat(0x5678, linhaarg);
            }*/
            else if (!strcmp(linhacomando,"LOADCFG") && iy == 7) {
                loadCFG(1);
                ix = 255;
            }
            else if (!strcmp(linhacomando,"MODE") && iy == 4) {
                // A definir
                ix = 255;
            }
            else if (!strcmp(linhacomando,"CAT") && iy == 3) {
                catFile((unsigned char*)linhaarg);
                ix = 255;
            }
            else {
                // Verifica se tem Arquivo com esse nome na pasta atual no disco
                ix = iy;
                linhacomando[ix] = '.';
                ix++;
                linhacomando[ix] = 'B';
                ix++;
                linhacomando[ix] = 'I';
                ix++;
                linhacomando[ix] = 'N';
                ix++;
                linhacomando[ix] = '\0';

                vretfat = ERRO_D_NOT_FOUND /*fsFindInDir(linhacomando, TYPE_FILE)*/ ;
                if (vretfat <= ERRO_D_START) {
                    // Se tiver, carrega em 0x01000000 e executa
                    loadFile((unsigned char*)linhacomando, (unsigned char*)vMemUserArea);
                    if (!verro)
                        runCmd();
                    else {
                        if (voutput == 1)
                            printf("Loading File Error...\n");
                        else {
                            message("Loading File Error...\0", BTCLOSE, 0);
                        }
                    }

                    ix = 255;
                }
                else {
                    // Se nao tiver, mostra erro
                    if (voutput == 1)
                        printf("Invalid Command or File Name\n");
                    else {
                        message("Invalid Command or\nFile Name\0", BTCLOSE, 0);
                    }
                    ix = 255;
                }
            }

            if (ix != 255)  {
                if (vpicret) {
                    /* ver como fazer */
                }

                if (((vpicret) && (vbytepic != RETURN_OK)) || ((!vpicret) && (vretfat != RETURN_OK))) {
                    printf("Command unsuccessfully\n\0");
                }
                else {
                    if (!strcmp(linhacomando,"CD")) {
                        if (linhaarg[0] == '.' && linhaarg[1] == '.') {
                            while (*vdiratup != '/') {
                                *vdiratup-- = '\0';
                            }

                            if (vdiratup > vdiratu)
                                *vdiratup = '\0';
                            else
                                vdiratup++;
                        }
                        else if(linhaarg[0] == '/') {
                            vdiratup = vdiratu;
                            *vdiratup++ = '/';
                            *vdiratup = '\0';
                       }
                        else if(linhaarg[0] != '.') {
                            vdiratup--;
                            if (*vdiratup++ != '/')
                                *vdiratup++ = '/';
                            for (varg = 0; varg < ix; varg++)
                                *vdiratup++ = linhaarg[varg];
                            *vdiratup = '\0';
                        }
                    }
                    else if (!strcmp(linhacomando,"DATE")) {
                        for(ix = 0; ix <= 9; ix++) {
                            /* ver como fazer */
                            vlinha[ix] = vbytepic;
                        }

                        vlinha[ix] = '\0';
                        printf("  Date is %s\n", vlinha);
                    }
                    else if (!strcmp(linhacomando,"TIME")) {
                        for(ix = 0; ix <= 7; ix++) {
                            /* ver como fazer */
                            vlinha[ix] = vbytepic;
                        }

                        vlinha[ix] = '\0';
                        printf("  Time is %n\n", vlinha);
                    }
                    else if (!strcmp(linhacomando,"IFCONFIG")) {
                        for(iy = 1; iy <= 5; iy++) {
                            for(ix = 0; ix <= 14; ix++) {
                                /* ver como fazer */
                                vlinha[ix] = vbytepic;
                            }
                            vlinha[ix] = '\0';
                            switch (iy) {
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
/*                    else if (!strcmp(linhacomando,"FORMAT")) {
                        if (voutput == 1)
                            writes("Format disk was successfully\n\0", vcorf, vcorb);
                        else {
                            message("Format disk was successfully\0", BTCLOSE, 0);
                        }
                    }*/
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void clearScr(unsigned int pcolor) {
    unsigned char ix, iy;

    #ifdef __USE_TFT_VDG__
        paramVDG[0] = 3;
        paramVDG[1] = 0xD0;
        paramVDG[2] = pcolor >> 8;
        paramVDG[3] = pcolor;
        commVDG(paramVDG);

        locate(0, 0, REPOS_CURSOR);
    #endif
}

//-----------------------------------------------------------------------------
void putPrompt(unsigned int plinadd) {
    if (plinadd)
        vlin = vlin + 1;

    locate(0,vlin, NOREPOS_CURSOR);

    printf("#%s>",(char*)vdiratu);

    vinip = vcol;
}

//-----------------------------------------------------------------------------
void verifyMGI(void) {
    if (voutput == 2 && vxmaxold == 0) {
        // Salva valores anteriores de vxmax e vymax, pcol e plin
        vxmaxold = vxmax;
        vymaxold = vymax;

        // Abre janela e define novos valores limites para pcol e plin, vxmax e vymax
        vxmax = 34;
        vymax = 14;
        vcol = 0;
        vlin = 0;
        voverx = 22;
        vovery = 36;

        strcpy((char*)vparamstr,"MMSJDOS");
        vparam[0] = 20;
        vparam[1] = 20;
        vparam[2] = 280;
        vparam[3] = 180;
        vparam[4] = BTCLOSE;
        showWindow();
    }
}

//-----------------------------------------------------------------------------
void writechar(int c, void *stream)
{
    writec((char*)c, vcorf, vcorb, ADD_POS_SCR);
}

//-----------------------------------------------------------------------------
void writestr(char *msgs)
{
    writes(msgs, vcorf, vcorb);
}

//-----------------------------------------------------------------------------
void writes(char *msgs, unsigned int pcolor, unsigned int pbcolor) {
    unsigned char ix = 10, iy, ichange = 0;
    unsigned char *ss = (unsigned char*)msgs;
    unsigned int xcolor = pcolor, xbcolor = pbcolor;

    #ifdef __USE_TFT_VDG__
        verifyMGI();

        if (voutput == 2) {
            xcolor = vcorwf;
            xbcolor = vcorwb;
        }

        while (*ss) {
          if (*ss >= 0x20)
              ix++;
          else
              ichange = 1;

          if ((vcol + (ix - 10)) > vxmax)
              ichange = 2;

          ss++;

          if (!*ss && !ichange)
             ichange = 3;

          if (ichange) {
             // Manda Sequencia de Controle
             if (ix > 10) {
                paramVDG[0] = ix;
                paramVDG[1] = 0xD1;
                paramVDG[2] = ((vcol * 8) + voverx) >> 8;
                paramVDG[3] = (vcol * 8) + voverx;
                paramVDG[4] = ((vlin * 10) + vovery) >> 8;
                paramVDG[5] = (vlin * 10) + vovery;
                paramVDG[6] = 8;
                paramVDG[7] = xcolor >> 8;
                paramVDG[8] = xcolor;
                paramVDG[9] = xbcolor >> 8;
                paramVDG[10] = xbcolor;
             }

             if (ichange == 1)
                ix++;

             iy = 11;
             while (*msgs && iy <= ix) {
                if (*msgs >= 0x20 && *msgs <= 0x7F) {
                    paramVDG[iy] = *msgs;
                    paramVDG[iy + 1] = '\0';
                    vcol = vcol + 1;
                }
                else {
                    if (*msgs == 0x0D) {
                        vcol = 0;
                    }
                    else if (*msgs == 0x0A) {
                        vcol = 0;  // So para teste, depois tiro e coloco '\r' junto com '\n'
                        vlin = vlin + 1;
                    }

                    locate(vcol, vlin, NOREPOS_CURSOR);
                }

                msgs++;
                iy++;
            }
            commVDG(paramVDG);

            if (ichange == 2) {
                vcol = 0;
                vlin++;
                locate(vcol, vlin, NOREPOS_CURSOR);
            }

            ichange = 0;
            ix = 10;
          }
        }
    #endif

    #ifdef __USE_UART_MON__
        puts(msgs);
    #endif
}

//-----------------------------------------------------------------------------
void writec(unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor, unsigned char ptipo) {
    unsigned int xcolor = pcolor, xbcolor = pbcolor;

    #ifdef __USE_TFT_VDG__
        verifyMGI();

        if (voutput == 2) {
            xcolor = vcorwf;
            xbcolor = vcorwb;
        }

        if (pbyte != '\n')
        {
            paramVDG[0] = 0x0B;
            paramVDG[1] = 0xD2;
            paramVDG[2] = ((vcol * 8) + voverx) >> 8;
            paramVDG[3] = (vcol * 8) + voverx;
            paramVDG[4] = ((vlin * 10) + vovery) >> 8;
            paramVDG[5] = (vlin * 10) + vovery;
            paramVDG[6] = 8;
            paramVDG[7] = xcolor >> 8;
            paramVDG[8] = xcolor;
            paramVDG[9] = xbcolor >> 8;
            paramVDG[10] = xbcolor;
            paramVDG[11] = pbyte;
            commVDG(paramVDG);

            if (ptipo == ADD_POS_SCR) {
                vcol = vcol + 1;

                if ((vlin == (vymax - 1)) && (vcol == vxmax)) {
                    vcol = 0;
                    vlin++;
                }

                locate(vcol, vlin, REPOS_CURSOR_ON_CHANGE);
            }
        }
        else
        {
            vlin++;
            vcol = 0;

            locate(vcol, vlin, REPOS_CURSOR_ON_CHANGE);
        }
    #endif

    #ifdef __USE_UART_MON__
        putc((char*) pbyte);
    #endif
}

//-----------------------------------------------------------------------------
void locate(unsigned char pcol, unsigned char plin, unsigned char pcur) {
    unsigned int vend, ix, iy, ichange = 0;
    unsigned int vlcdf[16];
    unsigned int vbytepic = 0;
    unsigned int ij, ik;

    #ifdef __USE_TFT_VDG__
        if (pcol > vxmax) {
            pcol = 0;
            plin++;
            ichange = 1;
        }

        if (plin > vymax) {
            pcol = 0;
            plin = vymax;

            if (voutput == 1) {
                paramVDG[0] = 4;
                paramVDG[1] = 0xD9;
                paramVDG[2] = 10;    // qtd de linhas que ocupa um char na tela
                paramVDG[3] = vcorb >> 8;
                paramVDG[4] = vcorb;
                commVDG(paramVDG);
            }
            else {
                // Criar scroll dentro da janela, ou dar upgrade no scroll atual
                pcol = 0;
                plin = 0;
            }

            ichange = 1;
        }

        vcol = pcol;
        vlin = plin;

        if (pcur == REPOS_CURSOR || (pcur == REPOS_CURSOR_ON_CHANGE && ichange)) {
            if (voutput == 1)
                writec(0x08, vcorf, vcorb, ADD_POS_SCR);
            vcol = vcol - 1;
        }
    #endif
}

//-----------------------------------------------------------------------------
unsigned long loadFile(unsigned char *parquivo, unsigned char* xaddress)
{
    unsigned short cc, dd;
    unsigned char vbuffer[128];
    unsigned char vbytegrava = 0;
    unsigned short xdado = 0, xcounter = 0;
    unsigned short vcrc, vcrcpic, vloop;
    unsigned long vsizeR, vsizefile = 0;

	vsizefile = 0;
    verro = 0;

    if (fsOpenFile((char*)parquivo) == RETURN_OK) {
		while (1) {
			vsizeR = fsReadFile((char*)parquivo, vsizefile, vbuffer, 128);

			if (vsizeR != 0) {
                for (dd = 00; dd <= 127; dd += 1){
                	vbytegrava = vbuffer[dd];
                    *xaddress++ = vbytegrava;
                }

                vsizefile += 128;
			}
			else
				break;
		}

        // Fecha o Arquivo
    	fsCloseFile((char*)parquivo, 0);
    }
    else
        verro = 1;

    return vsizefile;
}

//-----------------------------------------------------------------------------
unsigned char loadCFG(unsigned char ptipo) {
    unsigned char vret = 1, vset[40], vparam[40], vigual, ipos, iy, iz;
    unsigned int ix, vval, vdec;
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
                    vcorf = vval;
                }
                else if (!strcmp((char*)vset,"BCOLOR") && vigual == 6) {
                    vval = atoi((char*)vparam);
                    vcorb = vval;
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
                    if (voutput == 1)
                        writes("Config file syntax error...\n\0", vcorf, vcorb);
                    else {
                        message("Config file syntax error...\0", BTCLOSE, 0);
                    }
                }

                break;
            }

            mcfgfileptr++;
        }

        if (ipos <= 40 && ptipo) {
            if (voutput == 1)
                writes("Settings applied successfully...\n\0", vcorf, vcorb);
            else {
                message("Settings applied\nsuccessfully\0", BTCLOSE, 0);
            }
        }

        writec(0x08, vcorf, vcorb, NOADD_POS_SCR);
        ativaCursor();
    }
    else {
        if (ptipo) {
            if (voutput == 1)
                writes("Loading config file error...\n\0", vcorf, vcorb);
            else {
                message("Loading config file\nerror\0", BTCLOSE, 0);
            }
        }
        vret = 0;
    }

    return vret;
}

//-----------------------------------------------------------------------------
void catFile(unsigned char *parquivo) {
    unsigned int vbytepic;
    unsigned char *mcfgfileptr = (unsigned char*)mcfgfile, vqtd = 1;
    unsigned char *parqptr = (unsigned char*)parquivo;
    unsigned long vsizefile;

    while (*parqptr++)
        vqtd++;

    vsizefile = loadFile(parquivo, (unsigned char*)&mcfgfile);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco

    if (!verro) {
        while (vsizefile > 0) {
            if (*mcfgfileptr == 0x0D) {
                locate(0, vlin, NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x0A) {
                vlin = vlin + 1;
                locate(vcol, vlin, NOREPOS_CURSOR);
            }
            else if (*mcfgfileptr == 0x1A || *mcfgfileptr == 0x00) {
                break;
            }
            else {
                if (*mcfgfileptr >= 0x20 && *mcfgfileptr <= 0x7F)
                    writec(*mcfgfileptr, vcorf, vcorb, ADD_POS_SCR);
                else
                    writec(0x20, vcorf, vcorb, ADD_POS_SCR);
            }

            mcfgfileptr++;
            vsizefile--;
        }
    }
    else {
        if (voutput == 1)
            writes("Loading file error...\n\0", vcorf, vcorb);
        else {
            message("Loading file error...\0", BTCLOSE, 0);
        }
    }
}

//-----------------------------------------------------------------------------
// FAT32 Functions
//-----------------------------------------------------------------------------
unsigned char fsMountDisk(void)
{
    DSTATUS sta;
    unsigned char ixTimeout = 0;

    //  mailbox_emmc_clock(1);
    
    sta = disk_initialize(diskDrv);
    if (sta & STA_NOINIT) {
        return ERRO_B_OPEN_DISK;
    }

    // LER MBR
    ixTimeout = 4;
    while (1) 
    {
        ixTimeout--;
        if (disk_read(diskDrv, gDataBuffer, 0x00000000, 1) != RES_OK)
        {
            if (ixTimeout == 0)
                return ERRO_B_READ_DISK;
        }
        else
            break;
    }

    vdisk->firsts  = (((unsigned long)gDataBuffer[457] << 24) & 0xFF000000);
    vdisk->firsts |= (((unsigned long)gDataBuffer[456] << 16) & 0x00FF0000);
    vdisk->firsts |= (((unsigned long)gDataBuffer[455] << 8) & 0x0000FF00);
    vdisk->firsts |= ((unsigned long)gDataBuffer[454] & 0x000000FF);

    // LER FIRST CLUSTER
    if (disk_read(diskDrv, gDataBuffer, vdisk->firsts, 1) != RES_OK)
        return ERRO_B_READ_DISK;

    vdisk->reserv  = (unsigned int)gDataBuffer[15] << 8;
    vdisk->reserv |= (unsigned int)gDataBuffer[14];

    vdisk->fat = vdisk->reserv + vdisk->firsts;

    vdisk->sectorSize  = (unsigned long)gDataBuffer[12] << 8;
    vdisk->sectorSize |= (unsigned long)gDataBuffer[11];
    vdisk->SecPerClus = gDataBuffer[13];

    vdisk->fatsize  = (unsigned long)gDataBuffer[39] << 24;
    vdisk->fatsize |= (unsigned long)gDataBuffer[38] << 16;
    vdisk->fatsize |= (unsigned long)gDataBuffer[37] << 8;
    vdisk->fatsize |= (unsigned long)gDataBuffer[36];

    vdisk->root  = (unsigned long)gDataBuffer[47] << 24;
    vdisk->root |= (unsigned long)gDataBuffer[46] << 16;
    vdisk->root |= (unsigned long)gDataBuffer[45] << 8;
    vdisk->root |= (unsigned long)gDataBuffer[44];

    vdisk->type = FAT32;

    vdisk->data = vdisk->reserv + (2 * vdisk->fatsize);

    vclusterdir = vdisk->root;

/*    printf("\nfirsts: %x. ", vdisk->firsts);
    printf("\nreserv: %x. ", vdisk->reserv);
    printf("\nfat: %x. ", vdisk->fat);
    printf("\nsectorSize: %x. ", vdisk->sectorSize);
    printf("\nSecPerClus: %x. ", vdisk->SecPerClus);
    printf("\nfatsize: %x. ", vdisk->fatsize);
    printf("\nroot: %x. ", vdisk->root);
    printf("\ndata: %x. ", vdisk->data);*/

    return RETURN_OK;
}

void fsSetClusterDir (unsigned long vclusdiratu) {
    vclusterdir = vclusdiratu;
}

//-------------------------------------------------------------------------
unsigned long fsGetClusterDir (void) {
    return vclusterdir;
}

//-------------------------------------------------------------------------
unsigned char fsCreateFile(char * vfilename)
{
	// Verifica ja existe arquivo com esse nome
	if (fsFindInDir(vfilename, TYPE_ALL) < ERRO_D_START)
		return ERRO_B_FILE_FOUND;

	// Cria o arquivo com o nome especificado
	if (fsFindInDir(vfilename, TYPE_CREATE_FILE) >= ERRO_D_START)
		return ERRO_B_CREATE_FILE;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsOpenFile(char * vfilename)
{
	unsigned int vdirdate, vbytepic;
	unsigned char ds1307[7], ix, vlinha[12], vtemp[5];

	// Abre o arquivo especificado
	if (fsFindInDir(vfilename, TYPE_FILE) >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;

	// Ler Data/Hora do PIC
    /* ver como fazer
    sendPic(2);
    sendPic(picDOScmd);
    sendPic(picDOSdate);
    recPic();
    for(ix = 0; ix <= 9; ix++) {
        recPic();
        vlinha[ix] = vbytepic;
    }*/

    vtemp[0] = vlinha[0];
    vtemp[1] = vlinha[1];
    vtemp[2] = '\0';
    ds1307[3] = atoi((char*)vtemp);
    vtemp[0] = vlinha[3];
    vtemp[1] = vlinha[4];
    vtemp[2] = '\0';
    ds1307[4] = atoi((char*)vtemp);
    vtemp[0] = vlinha[6];
    vtemp[1] = vlinha[7];
    vtemp[2] = vlinha[8];
    vtemp[3] = vlinha[9];
    vtemp[4] = '\0';
    ds1307[5] = atoi((char*)vtemp);

	// Converte para a Data/Hora da FAT32
	vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

	// Grava nova data no lastaccess
	vdir->LastAccessDate  = vdirdate;

	if (fsUpdateDir() != RETURN_OK)
		return ERRO_B_UPDATE_DIR;

	return RETURN_OK;
}


//-------------------------------------------------------------------------
unsigned char fsCloseFile(char * vfilename, unsigned char vupdated)
{
	unsigned int vdirdate, vdirtime, vbytepic;
	unsigned char ds1307[7], vtemp[5], ix, vlinha[12];

	if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START) {
		if (vupdated) {
			// Ler Data/Hora do DS1307 - I2C
            /* ver como fazer
            sendPic(2);
            sendPic(picDOScmd);
            sendPic(picDOSdate);
            recPic();
            for(ix = 0; ix <= 9; ix++) {
                recPic();
                vlinha[ix] = vbytepic;
            }*/

            vtemp[0] = vlinha[0];
            vtemp[1] = vlinha[1];
            vtemp[2] = '\0';
            ds1307[3] = atoi((char*)vtemp);
            vtemp[0] = vlinha[3];
            vtemp[1] = vlinha[4];
            vtemp[2] = '\0';
            ds1307[4] = atoi((char*)vtemp);
            vtemp[0] = vlinha[6];
            vtemp[1] = vlinha[7];
            vtemp[2] = vlinha[8];
            vtemp[3] = vlinha[9];
            vtemp[4] = '\0';
            ds1307[5] = atoi((char*)vtemp);

            /* ver como fazer
            sendPic(2);
            sendPic(picDOScmd);
            sendPic(picDOStime);
            recPic();
            for(ix = 0; ix <= 7; ix++) {
                recPic();
                vlinha[ix] = vbytepic;
            }*/

            vtemp[0] = vlinha[0];
            vtemp[1] = vlinha[1];
            vtemp[2] = '\0';
            ds1307[0] = atoi((char*)vtemp);
            vtemp[0] = vlinha[3];
            vtemp[1] = vlinha[4];
            vtemp[2] = '\0';
            ds1307[1] = atoi((char*)vtemp);
            vtemp[0] = vlinha[6];
            vtemp[1] = vlinha[7];
            vtemp[2] = '\0';
            ds1307[2] = atoi((char*)vtemp);

			// Converte para a Data/Hora da FAT32
			vdirtime = datetimetodir(ds1307[0], ds1307[1], ds1307[2], CONV_HORA);
			vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

			// Grava nova data no lastaccess e nova data/hora no update date/time
			vdir->LastAccessDate  = vdirdate;
			vdir->UpdateTime = vdirtime;
			vdir->UpdateDate = vdirdate;

			if (fsUpdateDir() != RETURN_OK)
				return ERRO_B_UPDATE_DIR;
		}
	}
	else
		return ERRO_B_NOT_FOUND;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned long fsInfoFile(char * vfilename, unsigned char vtype)
{
	unsigned long vinfo = ERRO_D_NOT_FOUND, vtemp;

	// retornar as informa?es conforme solicitado.
	if (fsFindInDir(vfilename, TYPE_FILE) < ERRO_D_START) {
		switch (vtype) {
			case INFO_SIZE:
				vinfo = vdir->Size;
				break;
			case INFO_CREATE:
			    vtemp = (vdir->CreateDate << 16) | vdir->CreateTime;
				vinfo = (vtemp);
				break;
			case INFO_UPDATE:
			    vtemp = (vdir->UpdateDate << 16) | vdir->UpdateTime;
				vinfo = (vtemp);
				break;
			case INFO_LAST:
				vinfo = vdir->LastAccessDate;
				break;
		}
	}
	else
		return ERRO_D_NOT_FOUND;

	return vinfo;
}

//-------------------------------------------------------------------------
unsigned char fsDelFile(char * vfilename)
{
	// Apaga o arquivo solicitado
	if (fsFindInDir(vfilename, TYPE_DEL_FILE) >= ERRO_D_START)
		return ERRO_B_APAGAR_ARQUIVO;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsRenameFile(char * vfilename, char * vnewname)
{
	unsigned long vclusterfile;
	unsigned int ikk;
	unsigned char ixx, iyy;

	// Verificar se nome j?nao existe
	vclusterfile = fsFindInDir(vnewname, TYPE_ALL);

	if (vclusterfile < ERRO_D_START)
		return ERRO_B_FILE_FOUND;

	// Procura arquivo a ser renomeado
	vclusterfile = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterfile >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;

	// Altera nome na estrutura vdir
	memset(vdir->Name, 0x20, 8);
	memset(vdir->Ext, 0x20, 3);

	iyy = 0;
	for (ixx = 0; ixx <= strlen(vnewname); ixx++) {
		if (vnewname[ixx] == '\0')
			break;
		else if (vnewname[ixx] == '.')
			iyy = 8;
		else {
			if (iyy <= 7)
				vdir->Name[iyy] = vnewname[ixx];
			else {
			    ikk = iyy - 8;
				vdir->Ext[ikk] = vnewname[ixx];
			}

			iyy++;
		}
	}

	// Altera o nome, as demais informacoes nao alteram
	if (fsUpdateDir() != RETURN_OK)
		return ERRO_B_UPDATE_DIR;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
// Rotina para escrever/ler no disco
//-------------------------------------------------------------------------
unsigned char fsRWFile(unsigned long vclusterini, unsigned long voffset, unsigned char *buffer, unsigned char vtype)
{
	unsigned long vdata, vclusternew, vfat;
	unsigned int vpos, vsecfat, voffsec, voffclus, vtemp1, vtemp2, ikk, ikj;

	// Calcula offset de setor e cluster
	voffsec = voffset / vdisk->sectorSize;
	voffclus = voffsec / vdisk->SecPerClus;
	vclusternew = vclusterini;

	// Procura o cluster onde esta o setor a ser lido
	for (vpos = 0; vpos < voffclus; vpos++) {
		// Em operacao de escrita, como vai mexer com disco, salva buffer no setor de swap
		if (vtype == OPER_WRITE) {
		    ikk = vdisk->fat - 1;
			if (!fsSectorWrite(ikk, buffer, FALSE))
				return ERRO_B_READ_DISK;
		}

		vclusternew = fsFindNextCluster(vclusterini, NEXT_FIND);

		// Se for leitura e o offset der dentro do ultimo cluster, sai
		if (vtype == OPER_READ && vclusternew == LAST_CLUSTER_FAT32)
			return RETURN_OK;

		// Se for gravacao e o offset der dentro do ultimo cluster, cria novo cluster
		if ((vtype == OPER_WRITE || vtype == OPER_READWRITE) && vclusternew == LAST_CLUSTER_FAT32) {
			// Calcula novo cluster livre
			vclusternew = fsFindClusterFree(FREE_USE);

			if (vclusternew == ERRO_D_DISK_FULL)
				return ERRO_B_DISK_FULL;

			// Procura Cluster atual para altera?o
			vsecfat = vclusterini / 128;
			vfat = vdisk->fat + vsecfat;

			if (!fsSectorRead(vfat, gDataBuffer))
				return ERRO_B_READ_DISK;

			// Grava novo cluster no cluster atual
			vpos = (vclusterini - (128 * vsecfat)) * 4;
			gDataBuffer[vpos] = (unsigned char)(vclusternew & 0xFF);
			ikk = vpos + 1;
			gDataBuffer[ikk] = (unsigned char)((vclusternew / 0x100) & 0xFF);
			ikk = vpos + 2;
			gDataBuffer[ikk] = (unsigned char)((vclusternew / 0x10000) & 0xFF);
			ikk = vpos + 3;
			gDataBuffer[ikk] = (unsigned char)((vclusternew / 0x1000000) & 0xFF);

			if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
				return ERRO_B_WRITE_DISK;
		}

		vclusterini = vclusternew;

		// Em operacao de escrita, como mexeu com disco, le o buffer salvo no setor swap
		if (vtype == OPER_WRITE) {
		    ikk = vdisk->fat - 1;
			if (!fsSectorRead(ikk, buffer))
				return ERRO_B_READ_DISK;
		}
	}

	// Posiciona no setor dentro do cluster para ler/gravar
	vtemp1 = ((vclusternew - 2) * vdisk->SecPerClus);
    #ifdef __USE_FAT32_SDDISK__
        vtemp2 = vdisk->firsts * 2;
    #else
    	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
    #endif
	vdata = vtemp1 + vtemp2;
	vtemp1 = (voffclus * vdisk->SecPerClus);
	vdata += voffsec - vtemp1;

	if (vtype == OPER_READ || vtype == OPER_READWRITE) {
		// Le o setor e coloca no buffer
		if (!fsSectorRead(vdata, buffer))
			return ERRO_B_READ_DISK;
	}
	else {
		// Grava o buffer no setor
		if (!fsSectorWrite(vdata, buffer, FALSE))
			return ERRO_B_WRITE_DISK;
	}

	return RETURN_OK;
}

//-------------------------------------------------------------------------
// Retorna um buffer de "vsize" (max 255) Bytes, a partir do "voffset".
//-------------------------------------------------------------------------
unsigned char fsReadFile(char * vfilename, unsigned long voffset, unsigned char *buffer, unsigned char vsizebuffer)
{
	unsigned char ix, iy, vsizebf = 0;
	unsigned char vsize, vsetor = 0, vsizeant = 0;
	unsigned int voffsec, vtemp, ikk, ikj;
	unsigned long vclusterini;

	vclusterini = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterini >= ERRO_D_START)
		return 0;	// Erro na abertura/Arquivo nao existe

	// Verifica se o offset eh maior que o tamanho do arquivo
	if (voffset > vdir->Size)
		return 0;

	// Verifica se offset vai precisar gravar mais de 1 setor (entre 2 setores)
	vtemp = voffset / vdisk->sectorSize;
	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

	if ((voffsec + vsizebuffer) > vdisk->sectorSize)
		vsetor = 1;

	for (ix = 0; ix <= vsetor; ix++) {
    	vtemp = voffset / vdisk->sectorSize;
    	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

		// Ler setor do offset
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_READ) != RETURN_OK)
			return vsizebf;

		// Verifica tamanho a ser gravado
		if ((voffsec + vsizebuffer) <= vdisk->sectorSize)
			vsize = vsizebuffer - vsizeant;
		else
			vsize = vdisk->sectorSize - voffsec;

		vsizebf += vsize;

		if (vsizebf > (vdir->Size - voffset))
			vsizebf = vdir->Size - voffset;

		// Retorna os dados no buffer
		for (iy = 0; iy < vsize; iy++) {
		    ikk = vsizeant + iy;
		    ikj = voffsec + iy;
			buffer[ikk] = gDataBuffer[ikj];
        }

		vsizeant = vsize;
		voffset += vsize;
	}

	return vsizebf;
}

//-------------------------------------------------------------------------
// buffer a ser gravado nao pode ter mais que 128 bytes
//-------------------------------------------------------------------------
unsigned char fsWriteFile(char * vfilename, unsigned long voffset, unsigned char *buffer, unsigned char vsizebuffer)
{
	unsigned char vsetor = 0, ix, iy;
	unsigned int vsize, vsizeant = 0;
	unsigned int voffsec, vtemp, ikk, ikj;
	unsigned long vclusterini;

	vclusterini = fsFindInDir(vfilename, TYPE_FILE);

	if (vclusterini >= ERRO_D_START)
		return ERRO_B_FILE_NOT_FOUND;	// Erro na abertura/Arquivo nao existe

	// Verifica se offset vai precisar gravar mais de 1 setor (entre 2 setores)
	vtemp = voffset / vdisk->sectorSize;
	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

	if ((voffsec + vsizebuffer) > vdisk->sectorSize)
		vsetor = 1;

	for (ix = 0; ix <= vsetor; ix++) {
    	vtemp = voffset / vdisk->sectorSize;
    	voffsec = (voffset - (vdisk->sectorSize * (vtemp)));

		// Ler setor do offset
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_READWRITE) != RETURN_OK)
			return ERRO_B_READ_FILE;

		// Verifica tamanho a ser gravado
		if ((voffsec + vsizebuffer) <= vdisk->sectorSize)
			vsize = vsizebuffer - vsizeant;
		else
			vsize = vdisk->sectorSize - voffsec;

		// Prepara buffer para grava?o
		for (iy = 0; iy < vsize; iy++) {
		    ikk = iy + voffsec;
		    ikj = vsizeant + iy;
			gDataBuffer[ikk] = buffer[ikj];
		}

		// Grava setor
		if (fsRWFile(vclusterini, voffset, gDataBuffer, OPER_WRITE) != RETURN_OK)
			return ERRO_B_WRITE_FILE;

		vsizeant = vsize;

		if (vsetor == 1)
			voffset += vsize;
	}

	if ((voffset + vsizebuffer) > vdir->Size) {
		vdir->Size = voffset + vsizebuffer;

		if (fsUpdateDir() != RETURN_OK)
			return ERRO_B_UPDATE_DIR;
	}

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsMakeDir(char * vdirname)
{
	// Verifica ja existe arquivo/dir com esse nome
	if (fsFindInDir(vdirname, TYPE_ALL) < ERRO_D_START)
		return ERRO_B_DIR_FOUND;

	// Cria o dir solicitado
	if (fsFindInDir(vdirname, TYPE_CREATE_DIR) >= ERRO_D_START)
		return ERRO_B_CREATE_DIR;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsChangeDir(char * vdirname)
{
	unsigned long vclusterdirnew;

	// Troca o diretorio conforme especificado
	if (vdirname[0] == '/')
		vclusterdirnew = 0x00000002;
	else
		vclusterdirnew	= fsFindInDir(vdirname, TYPE_DIRECTORY);

	if (vclusterdirnew >= ERRO_D_START)
		return ERRO_B_DIR_NOT_FOUND;

	// Coloca o novo diretorio como atual
	vclusterdir = vclusterdirnew;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsRemoveDir(char * vdirname)
{
	// Apaga o diretorio conforme especificado
	if (fsFindInDir(vdirname, TYPE_DEL_DIR) >= ERRO_D_START)
		return ERRO_B_DIR_NOT_FOUND;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsPwdDir(unsigned char *vdirpath) {
    if (vclusterdir == vdisk->root) {
        vdirpath[0] = '/';
        vdirpath[1] = '\0';
    }
    else {
        vdirpath[0] = 'o';
        vdirpath[1] = '\0';
    }

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned long fsFindInDir(char * vname, unsigned char vtype)
{
	unsigned long vfat, vdata, vclusterfile, vclusterdirnew, vclusteratual, vtemp1, vtemp2;
	unsigned char fnameName[9], fnameExt[4];
	unsigned int im, ix, iy, iz, vpos, vsecfat, ventrydir, ixold;
	unsigned int vdirdate, vdirtime, ikk, ikj, vtemp, vbytepic;
	unsigned char vcomp, iw, ds1307[7], iww, vtempt[5], vlinha[5];

	memset(fnameName, 0x20, 8);
	memset(fnameExt, 0x20, 3);

	if (vname != NULL) {
		if (vname[0] == '.' && vname[1] == '.') {
			fnameName[0] = vname[0];
			fnameName[1] = vname[1];
		}
		else if (vname[0] == '.') {
			fnameName[0] = vname[0];
		}
		else {
			iy = 0;
			for (ix = 0; ix <= strlen(vname); ix++) {
				if (vname[ix] == '\0')
					break;
				else if (vname[ix] == '.')
					iy = 8;
				else {
					for (iww = 0; iww <= 56; iww++) {
						if (strValidChars[iww] == vname[ix])
							break;
					}

					if (iww > 56)
						return ERRO_D_INVALID_NAME;

					if (iy <= 7)
						fnameName[iy] = vname[ix];
					else {
					    ikk = iy - 8;
						fnameExt[ikk] = vname[ix];
					}

					iy++;
				}
			}
		}
	}

	vfat = vdisk->fat;
	vtemp1 = ((vclusterdir - 2) * vdisk->SecPerClus);

    #ifdef __USE_FAT32_SDDISK__
        vtemp2 = vdisk->firsts * 2;
    #else
    	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
    #endif

	vdata = vtemp1 + vtemp2;
	vclusterfile = ERRO_D_NOT_FOUND;
	vclusterdirnew = vclusterdir;
	ventrydir = 0;

	while (vdata != LAST_CLUSTER_FAT32) {
		for (iw = 0; iw < vdisk->SecPerClus; iw++) {

      		if (!fsSectorRead(vdata, gDataBuffer))
				return ERRO_D_READ_DISK;

			for (ix = 0; ix < vdisk->sectorSize; ix += 32) {
				for (iy = 0; iy < 8; iy++) {
				    ikk = ix + iy;
					vdir->Name[iy] = gDataBuffer[ikk];
				}

				for (iy = 0; iy < 3; iy++) {
				    ikk = ix + 8 + iy;
					vdir->Ext[iy] = gDataBuffer[ikk];
                }

                ikk = ix + 11;
				vdir->Attr = gDataBuffer[ikk];

                ikk = ix + 15;
				vdir->CreateTime  = (unsigned int)gDataBuffer[ikk] << 8;
                ikk = ix + 14;
				vdir->CreateTime |= (unsigned int)gDataBuffer[ikk];

                ikk = ix + 17;
				vdir->CreateDate  = (unsigned int)gDataBuffer[ikk] << 8;
                ikk = ix + 16;
				vdir->CreateDate |= (unsigned int)gDataBuffer[ikk];

                ikk = ix + 19;
				vdir->LastAccessDate  = (unsigned int)gDataBuffer[ikk] << 8;
                ikk = ix + 18;
				vdir->LastAccessDate |= (unsigned int)gDataBuffer[ikk];

                ikk = ix + 23;
				vdir->UpdateTime  = (unsigned int)gDataBuffer[ikk] << 8;
                ikk = ix + 22;
				vdir->UpdateTime |= (unsigned int)gDataBuffer[ikk];

                ikk = ix + 25;
				vdir->UpdateDate  = (unsigned int)gDataBuffer[ikk] << 8;
                ikk = ix + 24;
				vdir->UpdateDate |= (unsigned int)gDataBuffer[ikk];

                ikk = ix + 21;
				vdir->FirstCluster  = (unsigned long)gDataBuffer[ikk] << 24;
                ikk = ix + 20;
				vdir->FirstCluster |= (unsigned long)gDataBuffer[ikk] << 16;
                ikk = ix + 27;
				vdir->FirstCluster |= (unsigned long)gDataBuffer[ikk] << 8;
                ikk = ix + 26;
				vdir->FirstCluster |= (unsigned long)gDataBuffer[ikk];

                ikk = ix + 31;
				vdir->Size  = (unsigned long)gDataBuffer[ikk] << 24;
                ikk = ix + 30;
				vdir->Size |= (unsigned long)gDataBuffer[ikk] << 16;
                ikk = ix + 29;
				vdir->Size |= (unsigned long)gDataBuffer[ikk] << 8;
                ikk = ix + 28;
				vdir->Size |= (unsigned long)gDataBuffer[ikk];

				vdir->DirClusSec = vdata;
				vdir->DirEntry = ix;

				if (vtype == TYPE_FIRST_ENTRY) 
                {
					if (vdir->Name[0] != DIR_DEL) 
                    {
						if (vdir->Name[0] != DIR_EMPTY) 
                        {
                            if (vdir->Attr != ATTR_LONG_NAME && vdir->Attr != ATTR_DIR_SYSTEM)
							{
                                vclusterfile = vdir->FirstCluster;
    						    vdata = LAST_CLUSTER_FAT32;
    						    break;
                            }
    					}
					}
				}

				if (vtype == TYPE_EMPTY_ENTRY || vtype == TYPE_CREATE_FILE || vtype == TYPE_CREATE_DIR) {
					if (vdir->Name[0] == DIR_EMPTY || vdir->Name[0] == DIR_DEL) {
						vclusterfile = ventrydir;

						if (vtype != TYPE_EMPTY_ENTRY) {
							vclusterfile = fsFindClusterFree(FREE_USE);

							if (vclusterfile >= ERRO_D_START)
								return ERRO_D_NOT_FOUND;

						    if (!fsSectorRead(vdata, gDataBuffer))
								return ERRO_D_READ_DISK;

							for (iz = 0; iz <= 10; iz++) {
								if (iz <= 7) {
								    ikk = ix + iz;
									gDataBuffer[ikk] = fnameName[iz];
								}
								else {
								    ikk = ix + iz;
								    ikj = iz - 8;
									gDataBuffer[ikk] = fnameExt[ikj];
								}
							}

							if (vtype == TYPE_CREATE_FILE)
								gDataBuffer[ix + 11] = 0x00;
							else
								gDataBuffer[ix + 11] = ATTR_DIRECTORY;

							// Ler Data/Hora do DS1307 - I2C
                            /* ver como fazer
                            sendPic(2);
                            sendPic(picDOScmd);
                            sendPic(picDOSdate);
                            recPic();
                            for(im = 0; im <= 9; im++) {
                                recPic();
                                vlinha[im] = vbytepic;
                            }*/

                            vtempt[0] = vlinha[0];
                            vtempt[1] = vlinha[1];
                            vtempt[2] = '\0';
                            ds1307[3] = atoi((char*)vtempt);
                            vtempt[0] = vlinha[3];
                            vtempt[1] = vlinha[4];
                            vtempt[2] = '\0';
                            ds1307[4] = atoi((char*)vtempt);
                            vtempt[0] = vlinha[6];
                            vtempt[1] = vlinha[7];
                            vtempt[2] = vlinha[8];
                            vtempt[3] = vlinha[9];
                            vtempt[4] = '\0';
                            ds1307[5] = atoi((char*)vtempt);

                            /* ver como fazer
                            sendPic(2);
                            sendPic(picDOScmd);
                            sendPic(picDOStime);
                            recPic();
                            for(im = 0; im <= 7; im++) {
                                recPic();
                                vlinha[im] = vbytepic;
                            }*/

                            vtempt[0] = vlinha[0];
                            vtempt[1] = vlinha[1];
                            vtempt[2] = '\0';
                            ds1307[0] = atoi((char*)vtempt);
                            vtempt[0] = vlinha[3];
                            vtempt[1] = vlinha[4];
                            vtempt[2] = '\0';
                            ds1307[1] = atoi((char*)vtempt);
                            vtempt[0] = vlinha[6];
                            vtempt[1] = vlinha[7];
                            vtempt[2] = '\0';
                            ds1307[2] = atoi((char*)vtempt);

						    // Converte para a Data/Hora da FAT32
							vdirtime = datetimetodir(ds1307[0], ds1307[1], ds1307[2], CONV_HORA);
							vdirdate = datetimetodir(ds1307[3], ds1307[4], ds1307[5], CONV_DATA);

							// Coloca dados no buffer para gravacao
							ikk = ix + 12;
							gDataBuffer[ikk] = 0x00;	// case
							ikk = ix + 13;
							gDataBuffer[ikk] = 0x00;	// creation time in ms
							ikk = ix + 14;
							gDataBuffer[ikk] = (unsigned char)(vdirtime & 0xFF);	// creation time (ds1307)
							ikk = ix + 15;
							gDataBuffer[ikk] = (unsigned char)((vdirtime >> 8) & 0xFF);
							ikk = ix + 16;
							gDataBuffer[ikk] = (unsigned char)(vdirdate & 0xFF);	// creation date (ds1307)
							ikk = ix + 17;
							gDataBuffer[ikk] = (unsigned char)((vdirdate >> 8) & 0xFF);
							ikk = ix + 18;
							gDataBuffer[ikk] = (unsigned char)(vdirdate & 0xFF);	// last access	(ds1307)
							ikk = ix + 19;
							gDataBuffer[ikk] = (unsigned char)((vdirdate >> 8) & 0xFF);

							ikk = ix + 22;
							gDataBuffer[ikk] = (unsigned char)(vdirtime & 0xFF);	// time update (ds1307)
							ikk = ix + 23;
							gDataBuffer[ikk] = (unsigned char)((vdirtime >> 8) & 0xFF);
							ikk = ix + 24;
							gDataBuffer[ikk] = (unsigned char)(vdirdate & 0xFF);	// date update (ds1307)
							ikk = ix + 25;
							gDataBuffer[ikk] = (unsigned char)((vdirdate >> 8) & 0xFF);

							ikk = ix + 26;
						    gDataBuffer[ikk] = (unsigned char)(vclusterfile & 0xFF);
							ikk = ix + 27;
						    gDataBuffer[ikk] = (unsigned char)((vclusterfile / 0x100) & 0xFF);
							ikk = ix + 20;
						    gDataBuffer[ikk] = (unsigned char)((vclusterfile / 0x10000) & 0xFF);
							ikk = ix + 21;
						    gDataBuffer[ikk] = (unsigned char)((vclusterfile / 0x1000000) & 0xFF);

							ikk = ix + 28;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 29;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 30;
							gDataBuffer[ikk] = 0x00;
							ikk = ix + 31;
							gDataBuffer[ikk] = 0x00;

							if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
								return ERRO_D_WRITE_DISK;

							if (vtype == TYPE_CREATE_DIR) {
	  							// Posicionar na nova posicao do diretorio
                            	vtemp1 = ((vclusterfile - 2) * vdisk->SecPerClus);
                                #ifdef __USE_FAT32_SDDISK__
                                    vtemp2 = vdisk->firsts * 2;
                                #else
                                	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                                #endif
                            	vdata = vtemp1 + vtemp2;

								// Limpar novo cluster do diretorio (Zerar)
								memset(gDataBuffer, 0x00, vdisk->sectorSize);

								for (iz = 0; iz < vdisk->SecPerClus; iz++) {
								    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
										return ERRO_D_WRITE_DISK;
									vdata++;
								}

                            	vtemp1 = ((vclusterfile - 2) * vdisk->SecPerClus);
                                #ifdef __USE_FAT32_SDDISK__
                                    vtemp2 = vdisk->firsts * 2;
                                #else
                                	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                                #endif
                            	vdata = vtemp1 + vtemp2;

	  							// Criar diretorio . (atual)
	  							memset(gDataBuffer, 0x00, vdisk->sectorSize);

	  							ix = 0;
	  							gDataBuffer[0] = '.';
	  							gDataBuffer[1] = 0x20;
	  							gDataBuffer[2] = 0x20;
	  							gDataBuffer[3] = 0x20;
	  							gDataBuffer[4] = 0x20;
	  							gDataBuffer[5] = 0x20;
	  							gDataBuffer[6] = 0x20;
	  							gDataBuffer[7] = 0x20;
	  							gDataBuffer[8] = 0x20;
	  							gDataBuffer[9] = 0x20;
	  							gDataBuffer[10] = 0x20;

	  							gDataBuffer[11] = 0x10;

								gDataBuffer[12] = 0x00;	// case
								gDataBuffer[13] = 0x00;	// creation time in ms
								gDataBuffer[14] = (unsigned char)(vdirtime & 0xFF);	// creation time (ds1307)
								gDataBuffer[15] = (unsigned char)((vdirtime >> 8) & 0xFF);
								gDataBuffer[16] = (unsigned char)(vdirdate & 0xFF);	// creation date (ds1307)
								gDataBuffer[17] = (unsigned char)((vdirdate >> 8) & 0xFF);
								gDataBuffer[18] = (unsigned char)(vdirdate & 0xFF);	// last access	(ds1307)
								gDataBuffer[19] = (unsigned char)((vdirdate >> 8) & 0xFF);

								gDataBuffer[22] = (unsigned char)(vdirtime & 0xFF);	// time update (ds1307)
								gDataBuffer[23] = (unsigned char)((vdirtime >> 8) & 0xFF);
								gDataBuffer[24] = (unsigned char)(vdirdate & 0xFF);	// date update (ds1307)
								gDataBuffer[25] = (unsigned char)((vdirdate >> 8) & 0xFF);

	  						    gDataBuffer[26] = (unsigned char)(vclusterfile & 0xFF);
	  						    gDataBuffer[27] = (unsigned char)((vclusterfile / 0x100) & 0xFF);
	  						    gDataBuffer[20] = (unsigned char)((vclusterfile / 0x10000) & 0xFF);
	  						    gDataBuffer[21] = (unsigned char)((vclusterfile / 0x1000000) & 0xFF);

	  							gDataBuffer[28] = 0x00;
	  							gDataBuffer[29] = 0x00;
	  							gDataBuffer[30] = 0x00;
	  							gDataBuffer[31] = 0x00;

	  							// Criar diretorio .. (anterior)
	  							ix = 32;
	  							gDataBuffer[32] = '.';
	  							gDataBuffer[33] = '.';
	  							gDataBuffer[34] = 0x20;
	  							gDataBuffer[35] = 0x20;
	  							gDataBuffer[36] = 0x20;
	  							gDataBuffer[37] = 0x20;
	  							gDataBuffer[38] = 0x20;
	  							gDataBuffer[39] = 0x20;
	  							gDataBuffer[40] = 0x20;
	  							gDataBuffer[41] = 0x20;
	  							gDataBuffer[42] = 0x20;

	  							gDataBuffer[43] = 0x10;

								gDataBuffer[44] = 0x00;	// case
								gDataBuffer[45] = 0x00;	// creation time in ms
								gDataBuffer[46] = (unsigned char)(vdirtime & 0xFF);	// creation time (ds1307)
								gDataBuffer[47] = (unsigned char)((vdirtime >> 8) & 0xFF);
								gDataBuffer[48] = (unsigned char)(vdirdate & 0xFF);	// creation date (ds1307)
								gDataBuffer[49] = (unsigned char)((vdirdate >> 8) & 0xFF);
								gDataBuffer[50] = (unsigned char)(vdirdate & 0xFF);	// last access	(ds1307)
								gDataBuffer[51] = (unsigned char)((vdirdate >> 8) & 0xFF);

								gDataBuffer[54] = (unsigned char)(vdirtime & 0xFF);	// time update (ds1307)
								gDataBuffer[55] = (unsigned char)((vdirtime >> 8) & 0xFF);
								gDataBuffer[56] = (unsigned char)(vdirdate & 0xFF);	// date update (ds1307)
								gDataBuffer[57] = (unsigned char)((vdirdate >> 8) & 0xFF);

	  						    gDataBuffer[58] = (unsigned char)(vclusterdir & 0xFF);
	  						    gDataBuffer[59] = (unsigned char)((vclusterdir / 0x100) & 0xFF);
	  						    gDataBuffer[52] = (unsigned char)((vclusterdir / 0x10000) & 0xFF);
	  						    gDataBuffer[53] = (unsigned char)((vclusterdir / 0x1000000) & 0xFF);

	  							gDataBuffer[60] = 0x00;
	  							gDataBuffer[61] = 0x00;
	  							gDataBuffer[62] = 0x00;
	  							gDataBuffer[63] = 0x00;

	  						    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
	  								return ERRO_D_WRITE_DISK;
	              			}

							vdata = LAST_CLUSTER_FAT32;
							break;
						}

						vdata = LAST_CLUSTER_FAT32;
						break;
					}
				}
				else if (vtype != TYPE_FIRST_ENTRY) {
					if (vdir->Name[0] != DIR_EMPTY && vdir->Name[0] != DIR_DEL && vdir->Attr != ATTR_LONG_NAME && vdir->Attr != ATTR_DIR_SYSTEM) {
						vcomp = 1;
						for (iz = 0; iz <= 10; iz++) {
							if (iz <= 7) {
								if (fnameName[iz] != vdir->Name[iz]) {
									vcomp = 0;
									break;
								}
							}
							else {
							    ikk = iz - 8;
								if (fnameExt[ikk] != vdir->Ext[ikk]) {
									vcomp = 0;
									break;
								}
							}
						}

						if (vcomp) {
							if (vtype == TYPE_ALL || (vtype == TYPE_FILE && vdir->Attr != ATTR_DIRECTORY) || (vtype == TYPE_DIRECTORY && vdir->Attr == ATTR_DIRECTORY)) {
		  						vclusterfile = vdir->FirstCluster;
		  						break;
	  						}
	  						else if (vtype == TYPE_NEXT_ENTRY) {
		  						vtype = TYPE_FIRST_ENTRY;
		  					}
	  						else if (vtype == TYPE_DEL_FILE || vtype == TYPE_DEL_DIR) {
								// Guardando Cluster Atual
								vclusteratual = vdir->FirstCluster;

		  						// Apagando no Diretorio
		                		gDataBuffer[ix] = DIR_DEL;
		                		ikk = ix + 26;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 27;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 20;
								gDataBuffer[ikk] = 0x00;
		                		ikk = ix + 21;
								gDataBuffer[ikk] = 0x00;

								if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
		          			  		return ERRO_D_WRITE_DISK;

				                // Apagando vestigios na FAT
	          					while (1) {
				                    // Procura Proximo Cluster e ja zera
			           			    vclusterdirnew = fsFindNextCluster(vclusteratual, NEXT_FREE);

					                if (vclusterdirnew >= ERRO_D_START)
					                    return ERRO_D_NOT_FOUND;

					                if (vclusterdirnew == LAST_CLUSTER_FAT32) {
						                vclusterfile = LAST_CLUSTER_FAT32;
						          		vdata = LAST_CLUSTER_FAT32;
						          		break;
					                }

			            			// Tornar cluster atual o proximo
			            			vclusteratual = vclusterdirnew;
	          					}
	  						}
						}
					}
				}

				if (vdir->Name[0] == DIR_EMPTY) {
					vdata = LAST_CLUSTER_FAT32;
					break;
				}
			}

			if (vclusterfile < ERRO_D_START || vdata == LAST_CLUSTER_FAT32)
				break;

			ventrydir++;
			vdata++;
		}

		// Se conseguiu concluir a operacao solicitada, sai do loop
		if (vclusterfile < ERRO_D_START || vdata == LAST_CLUSTER_FAT32)
			break;
		else {
			// Posiciona na FAT, o endereco da pasta atual
			vsecfat = vclusterdirnew / 128;
			vfat = vdisk->fat + vsecfat;

		    if (!fsSectorRead(vfat, gDataBuffer))
				return ERRO_D_READ_DISK;

            vtemp = vclusterdirnew - (128 * vsecfat);
			vpos = vtemp * 4;
            ikk = vpos + 3;
			vclusterdirnew  = (unsigned long)gDataBuffer[ikk] << 24;
            ikk = vpos + 2;
			vclusterdirnew |= (unsigned long)gDataBuffer[ikk] << 16;
            ikk = vpos + 1;
			vclusterdirnew |= (unsigned long)gDataBuffer[ikk] << 8;
            ikk = vpos;
			vclusterdirnew |= (unsigned long)gDataBuffer[ikk];

			if (vclusterdirnew != LAST_CLUSTER_FAT32) {
				// Devolve a proxima posicao para procura/uso
            	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
                #ifdef __USE_FAT32_SDDISK__
                    vtemp2 = vdisk->firsts * 2;
                #else
                	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                #endif
            	vdata = vtemp1 + vtemp2;
			}
			else {
				// Se for para criar uma nova entrada no diretorio e nao tem mais espaco
				// Cria uma nova entrada na Fat
				if (vtype == TYPE_EMPTY_ENTRY || vtype == TYPE_CREATE_FILE || vtype == TYPE_CREATE_DIR) {
					vclusterdirnew = fsFindClusterFree(FREE_USE);

					if (vclusterdirnew < ERRO_D_START) {
					    if (!fsSectorRead(vfat, gDataBuffer))
							return ERRO_D_READ_DISK;

					    gDataBuffer[vpos] = (unsigned char)(vclusterdirnew & 0xFF);
					    ikk = vpos + 1;
					    gDataBuffer[ikk] = (unsigned char)((vclusterdirnew / 0x100) & 0xFF);
					    ikk = vpos + 2;
					    gDataBuffer[ikk] = (unsigned char)((vclusterdirnew / 0x10000) & 0xFF);
					    ikk = vpos + 3;
					    gDataBuffer[ikk] = (unsigned char)((vclusterdirnew / 0x1000000) & 0xFF);

					    if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
							return ERRO_D_WRITE_DISK;

						// Posicionar na nova posicao do diretorio
                    	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
                        #ifdef __USE_FAT32_SDDISK__
                            vtemp2 = vdisk->firsts * 2;
                        #else
                        	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                        #endif
                    	vdata = vtemp1 + vtemp2;

						// Limpar novo cluster do diretorio (Zerar)
						memset(gDataBuffer, 0x00, vdisk->sectorSize);

						for (iz = 0; iz < vdisk->SecPerClus; iz++) {
						    if (!fsSectorWrite(vdata, gDataBuffer, FALSE))
								return ERRO_D_WRITE_DISK;
							vdata++;
						}

                    	vtemp1 = ((vclusterdirnew - 2) * vdisk->SecPerClus);
                        #ifdef __USE_FAT32_SDDISK__
                            vtemp2 = vdisk->firsts * 2;
                        #else
                        	vtemp2 = (vdisk->reserv + vdisk->firsts + (2 * vdisk->fatsize));
                        #endif
                    	vdata = vtemp1 + vtemp2;
					}
					else {
						vclusterdirnew = LAST_CLUSTER_FAT32;
						vclusterfile = ERRO_D_NOT_FOUND;
						vdata = vclusterdirnew;
					}
				}
				else {
					vdata = vclusterdirnew;
				}
			}
		}
	}

	return vclusterfile;
}

//-------------------------------------------------------------------------
unsigned char fsUpdateDir()
{
	unsigned char iy;
	unsigned int ventry, ikk;

	if (!fsSectorRead(vdir->DirClusSec, gDataBuffer))
		return ERRO_B_READ_DISK;

    ventry = vdir->DirEntry;

	for (iy = 0; iy < 8; iy++) {
	    ikk = ventry + iy;
		gDataBuffer[ikk] = vdir->Name[iy];
	}

	for (iy = 0; iy < 3; iy++) {
	    ikk = ventry + 8 + iy;
		gDataBuffer[ikk] = vdir->Ext[iy];
	}

    ikk = ventry + 18;
	gDataBuffer[ikk] = (unsigned char)(vdir->LastAccessDate & 0xFF);	// last access	(ds1307)
    ikk = ventry + 19;
	gDataBuffer[ikk] = (unsigned char)((vdir->LastAccessDate / 0x100) & 0xFF);

    ikk = ventry + 22;
	gDataBuffer[ikk] = (unsigned char)(vdir->UpdateTime & 0xFF);	// time update (ds1307)
    ikk = ventry + 23;
	gDataBuffer[ikk] = (unsigned char)((vdir->UpdateTime / 0x100) & 0xFF);

    ikk = ventry + 24;
	gDataBuffer[ikk] = (unsigned char)(vdir->UpdateDate & 0xFF);	// date update (ds1307)
    ikk = ventry + 25;
	gDataBuffer[ikk] = (unsigned char)((vdir->UpdateDate / 0x100) & 0xFF);

    ikk = ventry + 28;
    gDataBuffer[ikk] = (unsigned char)(vdir->Size & 0xFF);
    ikk = ventry + 29;
    gDataBuffer[ikk] = (unsigned char)((vdir->Size / 0x100) & 0xFF);
    ikk = ventry + 30;
    gDataBuffer[ikk] = (unsigned char)((vdir->Size / 0x10000) & 0xFF);
    ikk = ventry + 31;
    gDataBuffer[ikk] = (unsigned char)((vdir->Size / 0x1000000) & 0xFF);

    if (!fsSectorWrite(vdir->DirClusSec, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned long fsFindNextCluster(unsigned long vclusteratual, unsigned char vtype)
{
	unsigned long vfat, vclusternew;
	unsigned int vpos, vsecfat, ikk;

	vsecfat = vclusteratual / 128;
	vfat = vdisk->fat + vsecfat;

	if (!fsSectorRead(vfat, gDataBuffer))
		return ERRO_D_READ_DISK;

	vpos = (vclusteratual - (128 * vsecfat)) * 4;
	ikk = vpos + 3;
	vclusternew  = (unsigned long)gDataBuffer[ikk] << 24;
	ikk = vpos + 2;
	vclusternew |= (unsigned long)gDataBuffer[ikk] << 16;
	ikk = vpos + 1;
	vclusternew |= (unsigned long)gDataBuffer[ikk] << 8;
	vclusternew |= (unsigned long)gDataBuffer[vpos];

	if (vtype != NEXT_FIND) {
		if (vtype == NEXT_FREE) {
			gDataBuffer[vpos] = 0x00;
        	ikk = vpos + 1;
			gDataBuffer[ikk] = 0x00;
        	ikk = vpos + 2;
			gDataBuffer[ikk] = 0x00;
        	ikk = vpos + 3;
			gDataBuffer[ikk] = 0x00;
		}
		else if (vtype == NEXT_FULL) {
			gDataBuffer[vpos] = 0xFF;
        	ikk = vpos + 1;
			gDataBuffer[ikk] = 0xFF;
        	ikk = vpos + 2;
			gDataBuffer[ikk] = 0xFF;
        	ikk = vpos + 3;
			gDataBuffer[ikk] = 0x0F;
		}

		if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
			return ERRO_D_WRITE_DISK;
	}

  return vclusternew;
}

//-------------------------------------------------------------------------
unsigned long fsFindClusterFree(unsigned char vtype)
{
  	unsigned long vclusterfree = 0x00, cc, vfat;
	unsigned int jj, ikk, ikk2, ikk3;

	vfat = vdisk->fat;

	for (cc = 0; cc <= vdisk->fatsize; cc++) {
	    // LER FAT SECTOR
		if (!fsSectorRead(vfat, gDataBuffer))
			return ERRO_D_READ_DISK;

		// Procura Cluster Livre dentro desse setor
		for (jj = 0; jj < vdisk->sectorSize; jj += 4) {
		    ikk = jj + 1;
		    ikk2 = jj + 2;
		    ikk3 = jj + 3;
			if (gDataBuffer[jj] == 0x00 && gDataBuffer[ikk] == 0x00 && gDataBuffer[ikk2] == 0x00 && gDataBuffer[ikk3] == 0x00)
			    break;

			vclusterfree++;
		}

		// Se achou algum setor livre, sai do loop
		if (jj < vdisk->sectorSize)
			break;

		// Soma mais 1 para procurar proximo cluster
		vfat++;
	}

	if (cc > vdisk->fatsize)
		vclusterfree = ERRO_D_DISK_FULL;
	else {
		if (vtype == FREE_USE) {
		    gDataBuffer[jj] = 0xFF;
		    ikk = jj + 1;
		    gDataBuffer[ikk] = 0xFF;
		    ikk = jj + 2;
		    gDataBuffer[ikk] = 0xFF;
		    ikk = jj + 3;
		    gDataBuffer[ikk] = 0x0F;

		    if (!fsSectorWrite(vfat, gDataBuffer, FALSE))
				return ERRO_D_WRITE_DISK;
		}
	}

	return (vclusterfree);
}

//-------------------------------------------------------------------------
unsigned char fsFormat (long int serialNumber, char * volumeID)
{
    unsigned int    j;
    unsigned long   secCount, RootDirSectors;
    unsigned long   root, fat, firsts, fatsize, test;
    unsigned long   Index;
	unsigned char    SecPerClus;

    unsigned char *  dataBufferPointer = (unsigned char*)gDataBuffer;

	// Ler MBR
    if (!fsSectorRead(0x00, gDataBuffer))
		return ERRO_B_READ_DISK;

    secCount  = (unsigned long)gDataBuffer[461] << 24;
    secCount |= (unsigned long)gDataBuffer[460] << 16;
    secCount |= (unsigned long)gDataBuffer[459] << 8;
    secCount |= (unsigned long)gDataBuffer[458];

    firsts  = (unsigned long)gDataBuffer[457] << 24;
    firsts |= (unsigned long)gDataBuffer[456] << 16;
    firsts |= (unsigned long)gDataBuffer[455] << 8;
    firsts |= (unsigned long)gDataBuffer[454];

    *(dataBufferPointer + 450) = 0x0B;

    if (!fsSectorWrite (0x00, gDataBuffer, TRUE))
		return ERRO_B_WRITE_DISK;

	//-------------------

	if (secCount >= 0x000EEB7F && secCount <= 0x01000000)	// 512 MB to 8 GB, 8 sectors per cluster
		SecPerClus = 8;
	else if (secCount > 0x01000000 && secCount <= 0x02000000) // 8 GB to 16 GB, 16 sectors per cluster
		SecPerClus = 16;
	else if (secCount > 0x02000000 && secCount <= 0x04000000) // 16 GB to 32 GB, 32 sectors per cluster
		SecPerClus = 32;
	else if (secCount > 0x04000000) // More than 32 GB, 64 sectors per cluster
		SecPerClus = 64;
	//-------------------

	//-------------------
    fatsize = (secCount - 0x26);
    fatsize = (fatsize / ((256 * SecPerClus + 2) / 2));
    fat = 0x26 + firsts;
    root = fat + (2 * fatsize);
	//-------------------

	// Formata MicroSD
    memset (gDataBuffer, 0x00, MEDIA_SECTOR_SIZE);

    // Non-file system specific values
    gDataBuffer[0] = 0xEB;         //Jump instruction
    gDataBuffer[1] = 0x3C;
    gDataBuffer[2] = 0x90;
    gDataBuffer[3] =  'M';         //OEM Name
    gDataBuffer[4] =  'M';
    gDataBuffer[5] =  'S';
    gDataBuffer[6] =  'J';
    gDataBuffer[7] =  ' ';
    gDataBuffer[8] =  'F';
    gDataBuffer[9] =  'A';
    gDataBuffer[10] = 'T';

    gDataBuffer[11] = 0x00;             //Sector size
    gDataBuffer[12] = 0x02;

    gDataBuffer[13] = SecPerClus;   //Sectors per cluster

    gDataBuffer[14] = 0x26;         //Reserved sector count
    gDataBuffer[15] = 0x00;

	fat = 0x26 + firsts;

    gDataBuffer[16] = 0x02;         //number of FATs

    gDataBuffer[17] = 0x00;          //Max number of root directory entries - 512 files allowed
    gDataBuffer[18] = 0x00;

    gDataBuffer[19] = 0x00;         //total sectors
    gDataBuffer[20] = 0x00;

    gDataBuffer[21] = 0xF8;         //Media Descriptor

    gDataBuffer[22] = 0x00;         //Sectors per FAT
    gDataBuffer[23] = 0x00;

    gDataBuffer[24] = 0x3F;         //Sectors per track
    gDataBuffer[25] = 0x00;

    gDataBuffer[26] = 0xFF;         //Number of heads
    gDataBuffer[27] = 0x00;

    // Hidden sectors = sectors between the MBR and the boot sector
    gDataBuffer[28] = (unsigned char)(firsts & 0xFF);
    gDataBuffer[29] = (unsigned char)((firsts / 0x100) & 0xFF);
    gDataBuffer[30] = (unsigned char)((firsts / 0x10000) & 0xFF);
    gDataBuffer[31] = (unsigned char)((firsts / 0x1000000) & 0xFF);

    // Total Sectors = same as sectors in the partition from MBR
    gDataBuffer[32] = (unsigned char)(secCount & 0xFF);
    gDataBuffer[33] = (unsigned char)((secCount / 0x100) & 0xFF);
    gDataBuffer[34] = (unsigned char)((secCount / 0x10000) & 0xFF);
    gDataBuffer[35] = (unsigned char)((secCount / 0x1000000) & 0xFF);

	// Sectors per FAT
	gDataBuffer[36] = (unsigned char)(fatsize & 0xFF);
    gDataBuffer[37] = (unsigned char)((fatsize / 0x100) & 0xFF);
    gDataBuffer[38] = (unsigned char)((fatsize / 0x10000) & 0xFF);
    gDataBuffer[39] = (unsigned char)((fatsize / 0x1000000) & 0xFF);

    gDataBuffer[40] = 0x00;         //Active FAT
    gDataBuffer[41] = 0x00;

    gDataBuffer[42] = 0x00;         //File System version
    gDataBuffer[43] = 0x00;

    gDataBuffer[44] = 0x02;         //First cluster of the root directory
    gDataBuffer[45] = 0x00;
    gDataBuffer[46] = 0x00;
    gDataBuffer[47] = 0x00;

    gDataBuffer[48] = 0x01;         //FSInfo
    gDataBuffer[49] = 0x00;

    gDataBuffer[50] = 0x00;         //Backup Boot Sector
    gDataBuffer[51] = 0x00;

    gDataBuffer[52] = 0x00;         //Reserved for future expansion
    gDataBuffer[53] = 0x00;
    gDataBuffer[54] = 0x00;
    gDataBuffer[55] = 0x00;
    gDataBuffer[56] = 0x00;
    gDataBuffer[57] = 0x00;
    gDataBuffer[58] = 0x00;
    gDataBuffer[59] = 0x00;
    gDataBuffer[60] = 0x00;
    gDataBuffer[61] = 0x00;
    gDataBuffer[62] = 0x00;
    gDataBuffer[63] = 0x00;

    gDataBuffer[64] = 0x00;         // Physical drive number

    gDataBuffer[65] = 0x00;         // Reserved (current head)

    gDataBuffer[66] = 0x29;         // Signature code

    gDataBuffer[67] = (unsigned char)(serialNumber & 0xFF);
    gDataBuffer[68] = (unsigned char)((serialNumber / 0x100) & 0xFF);
    gDataBuffer[69] = (unsigned char)((serialNumber / 0x10000) & 0xFF);
    gDataBuffer[70] = (unsigned char)((serialNumber / 0x1000000) & 0xFF);

    // Volume ID
    if (volumeID != NULL)
    {
        for (Index = 0; (*(volumeID + Index) != 0) && (Index < 11); Index++)
        {
            gDataBuffer[Index + 71] = *(volumeID + Index);
        }
        while (Index < 11)
        {
            gDataBuffer[71 + Index++] = 0x20;
        }
    }
    else
    {
        for (Index = 0; Index < 11; Index++)
        {
            gDataBuffer[Index+71] = 0;
        }
    }

    gDataBuffer[82] = 'F';
    gDataBuffer[83] = 'A';
    gDataBuffer[84] = 'T';
    gDataBuffer[85] = '3';
    gDataBuffer[86] = '2';
    gDataBuffer[87] = ' ';
    gDataBuffer[88] = ' ';
    gDataBuffer[89] = ' ';

    gDataBuffer[510] = 0x55;
    gDataBuffer[511] = 0xAA;

	if (!fsSectorWrite(firsts, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

    // Erase the FAT
    memset (gDataBuffer, 0x00, MEDIA_SECTOR_SIZE);

    gDataBuffer[0] = 0xF8;          //BPB_Media byte value in its low 8 bits, and all other bits are set to 1
    gDataBuffer[1] = 0xFF;
    gDataBuffer[2] = 0xFF;
    gDataBuffer[3] = 0x0F;

    gDataBuffer[4] = 0xFF;          //Disk is clean and no read/write errors were encountered
    gDataBuffer[5] = 0xFF;
    gDataBuffer[6] = 0xFF;
    gDataBuffer[7] = 0xFF;

    gDataBuffer[8]  = 0xFF;         //Root Directory EOF
    gDataBuffer[9]  = 0xFF;
    gDataBuffer[10] = 0xFF;
    gDataBuffer[11] = 0x0F;

    for (j = 1; j != 0xFFFF; j--)
    {
        if (!fsSectorWrite (fat + (j * fatsize), gDataBuffer, FALSE))
			return ERRO_B_WRITE_DISK;
    }

    memset (gDataBuffer, 0x00, 12);

    for (Index = fat + 1; Index < (fat + fatsize); Index++)
    {
        for (j = 1; j != 0xFFFF; j--)
        {
            if (!fsSectorWrite (Index + (j * fatsize), gDataBuffer, FALSE))
				return ERRO_B_WRITE_DISK;
        }
    }

    // Erase the root directory
    for (Index = 1; Index < SecPerClus; Index++)
    {
        if (!fsSectorWrite (root + Index, gDataBuffer, FALSE))
			return ERRO_B_WRITE_DISK;
    }

    // Create a drive name entry in the root dir
    Index = 0;
    while ((*(volumeID + Index) != 0) && (Index < 11))
    {
        gDataBuffer[Index] = *(volumeID + Index);
        Index++;
    }
    while (Index < 11)
    {
        gDataBuffer[Index++] = ' ';
    }
    gDataBuffer[11] = 0x08;
    gDataBuffer[17] = 0x11;
    gDataBuffer[19] = 0x11;
    gDataBuffer[23] = 0x11;

    if (!fsSectorWrite (root, gDataBuffer, FALSE))
		return ERRO_B_WRITE_DISK;

	return RETURN_OK;
}

//-------------------------------------------------------------------------
unsigned char fsSectorRead(unsigned long vsector, unsigned char* vbuffer)
{
    DRESULT vstatus;

    vstatus = disk_read(diskDrv, vbuffer, vsector, 1);

    if (vstatus != RES_OK)
    {
        printf("Disk I/O Read Error (%d)\n",vstatus);
        return 0;
    }

    return 1;
}

//-------------------------------------------------------------------------
unsigned char fsSectorWrite(unsigned long vsector, unsigned char* vbuffer, unsigned char vtipo)
{
    DRESULT vstatus;

    vstatus = disk_write(diskDrv, vbuffer, vsector, 1);

    if (vstatus != RES_OK)
    {
        printf("Disk I/O Write Error (%d)\n",vstatus);
        return 0;
    }

    return 1;
}

//-----------------------------------------------------------------------------
// Graphic Interface Functions
//-----------------------------------------------------------------------------
void writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor) {
    unsigned char ix = 10, *ss = (unsigned char*)msgs;
    unsigned int iy;

    #ifdef __USE_TFT_VDG__
        while (*ss) {
          if (*ss >= 0x20)
              ix++;
          ss++;
        }

        // Manda Sequencia de Controle
        if (ix > 10) {
            paramVDG[0] = ix;
            paramVDG[1] = 0xD1;
            paramVDG[2] = x >> 8;
            paramVDG[3] = x;
            paramVDG[4] = y >> 8;
            paramVDG[5] = y;
            paramVDG[6] = sizef;
            paramVDG[7] = pcolor >> 8;
            paramVDG[8] = pcolor;
            paramVDG[9] = pbcolor >> 8;
            paramVDG[10] = pbcolor;

            iy = 11;
            while (*msgs) {
                if (*msgs >= 0x20 && *msgs <= 0x7F) 
                {
                    paramVDG[iy] = *msgs;
                    paramVDG[iy + 1] = '\0';
                    iy++;
                }
                msgs++;
            }

            commVDG(paramVDG);
        }
    #endif
}

//-----------------------------------------------------------------------------
void writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] = 0x0B;
        paramVDG[1] = 0xD2;
        paramVDG[2] = pposx >> 8;
        paramVDG[3] = pposx;
        paramVDG[4] = pposy >> 8;
        paramVDG[5] = pposy;
        paramVDG[6] = sizef;
        paramVDG[7] = pcolor >> 8;
        paramVDG[8] = pcolor;
        paramVDG[9] = pbcolor >> 8;
        paramVDG[10] = pbcolor;
        paramVDG[11] = pbyte;

        commVDG(paramVDG);

        pposx = pposx + sizef;

        if ((pposx + sizef) > vxgmax)
            pposx = pposx - sizef;
    #endif
}

//-----------------------------------------------------------------------------
void locatexy(unsigned int xx, unsigned int yy) {
    pposx = xx;
    pposy = yy;
}

//-----------------------------------------------------------------------------
void SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] = 10;
        paramVDG[1] = 0xEA;
        paramVDG[2] = yi >> 8;
        paramVDG[3] = yi;
        paramVDG[4] = xi >> 8;
        paramVDG[5] = xi;
        paramVDG[6] = pheight >> 8;
        paramVDG[7] = pheight;
        paramVDG[8] = pwidth >> 8;
        paramVDG[9] = pwidth;
        paramVDG[10] = pPage;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  10;
        paramVDG[1] =  0xEB;
        paramVDG[2] =  yi >> 8;
        paramVDG[3] =  yi;
        paramVDG[4] =  xi >> 8;
        paramVDG[5] =  xi;
        paramVDG[6] =  pheight >> 8;
        paramVDG[7] =  pheight;
        paramVDG[8] =  pwidth >> 8;
        paramVDG[9] =  pwidth;
        paramVDG[10] = pPage;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void SetDot(unsigned int x, unsigned int y, unsigned int color) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  8;
        paramVDG[1] =  0xD7;
        paramVDG[2] =  x >> 8;
        paramVDG[3] =  x;
        paramVDG[4] =  y >> 8;
        paramVDG[5] =  y;
        paramVDG[6] =  0;
        paramVDG[7] =  color >> 8;
        paramVDG[8] =  color;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor) {
    unsigned int xf, yf;

    #ifdef __USE_TFT_VDG__
        xf = (xi + pwidth);
        yf = (yi + pheight);

        paramVDG[0] =  11;
        paramVDG[1] =  0xD3;
        paramVDG[2] =  xi >> 8;
        paramVDG[3] =  xi;
        paramVDG[4] =  yi >> 8;
        paramVDG[5] =  yi;
        paramVDG[6] =  xf >> 8;
        paramVDG[7] =  xf;
        paramVDG[8] =  yf >> 8;
        paramVDG[9] =  yf;
        paramVDG[10] =  pcor >> 8;
        paramVDG[11] =  pcor;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  11;
        paramVDG[1] =  0xD4;
        paramVDG[2] =  x1 >> 8;
        paramVDG[3] =  x1;
        paramVDG[4] =  y1 >> 8;
        paramVDG[5] =  y1;
        paramVDG[6] =  x2 >> 8;
        paramVDG[7] =  x2;
        paramVDG[8] =  y2 >> 8;
        paramVDG[9] =  y2;
        paramVDG[10] =  color >> 8;
        paramVDG[11] =  color;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void DrawRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int color) {
    unsigned int xf, yf;

    #ifdef __USE_TFT_VDG__
        xf = (xi + pwidth);
        yf = (yi + pheight);

        paramVDG[0] =  11;
        paramVDG[1] =  0xD5;
        paramVDG[2] =  xi >> 8;
        paramVDG[3] =  xi;
        paramVDG[4] =  yi >> 8;
        paramVDG[5] =  yi;
        paramVDG[6] =  xf >> 8;
        paramVDG[7] =  xf;
        paramVDG[8] =  yf >> 8;
        paramVDG[9] =  yf;
        paramVDG[10] =  color >> 8;
        paramVDG[11] =  color;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void DrawRoundRect(void) {
    unsigned int xi, yi, pwidth, pheight, color;
	unsigned char radius;
	int tSwitch;
    unsigned int x1 = 0, y1, xt, yt, wt;

    #ifdef __USE_TFT_VDG__
        xi = vparam[0];
        yi = vparam[1];
        pwidth = vparam[2];
        pheight = vparam[3];
        radius = vparam[4];
        color = vparam[5];

        y1 = radius;

    	tSwitch = 3 - 2 * radius;

    	while (x1 <= y1) {
    	    xt = xi + radius - x1;
    	    yt = yi + radius - y1;
    	    SetDot(xt, yt, color);

    	    xt = xi + radius - y1;
    	    yt = yi + radius - x1;
    	    SetDot(xt, yt, color);

            xt = xi + pwidth-radius + x1;
    	    yt = yi + radius - y1;
    	    SetDot(xt, yt, color);

            xt = xi + pwidth-radius + y1;
    	    yt = yi + radius - x1;
    	    SetDot(xt, yt, color);

            xt = xi + pwidth-radius + x1;
            yt = yi + pheight-radius + y1;
    	    SetDot(xt, yt, color);

            xt = xi + pwidth-radius + y1;
            yt = yi + pheight-radius + x1;
    	    SetDot(xt, yt, color);

    	    xt = xi + radius - x1;
            yt = yi + pheight-radius + y1;
    	    SetDot(xt, yt, color);

    	    xt = xi + radius - y1;
            yt = yi + pheight-radius + x1;
    	    SetDot(xt, yt, color);

    	    if (tSwitch < 0) {
    	    	tSwitch += (4 * x1 + 6);
    	    } else {
    	    	tSwitch += (4 * (x1 - y1) + 10);
    	    	y1--;
    	    }
    	    x1++;
    	}

        xt = xi + radius;
        yt = yi + pheight;
        wt = pwidth - (2 * radius);
    	DrawHoriLine(xt, yi, wt, color);		// top
    	DrawHoriLine(xt, yt, wt, color);	// bottom

        xt = xi + pwidth;
        yt = yi + radius;
        wt = pheight - (2 * radius);
    	DrawVertLine(xi, yt, wt, color);		// left
    	DrawVertLine(xt, yt, wt, color);	// right
    #endif
}

//-----------------------------------------------------------------------------
void DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  10;
        paramVDG[1] =  0xD6;
        paramVDG[2] =  xi >> 8;
        paramVDG[3] =  xi;
        paramVDG[4] =  yi >> 8;
        paramVDG[5] =  yi;
        paramVDG[6] =  pang;
        paramVDG[7] =  pfil;
        paramVDG[8] =  0;
        paramVDG[9] =  pcor >> 8;
        paramVDG[10] =  pcor;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  9;
        paramVDG[1] =  0xEC;
        paramVDG[2] =  x >> 8;
        paramVDG[3] =  x;
        paramVDG[4] =  y >> 8;
        paramVDG[5] =  y;
        paramVDG[6] =  pheight >> 8;
        paramVDG[7] =  pheight;
        paramVDG[8] =  pwidth >> 8;
        paramVDG[9] =  pwidth;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    #ifdef __USE_TFT_VDG__
        DrawRect((x - 1), (y - 1), (pwidth + 2), (pheight + 2), Red);
    #endif
}

//-----------------------------------------------------------------------------
void PutImage(unsigned char* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    #ifdef __USE_TFT_VDG__
        /* ver como fazer */
    #endif
}

//-----------------------------------------------------------------------------
void PutIcone(unsigned char* vimage, unsigned int x, unsigned int y) {
    unsigned int ix, iy, iz, pw, ph;
    unsigned char* pimage;
    unsigned char ic, igh;

    #ifdef __USE_TFT_VDG__
        ic = 0;
        iz = 0;
        pw = 24;
        ph = 24;
        pimage = vimage;

        // Acumula dados, enviando em 9 vezes de 64 x 16 bits
        paramVDG[0] =  130;
        paramVDG[1] =  0xDE;
        paramVDG[2] =  ic;
        igh = 3;
    	for (ix = 0; ix < 576; ix++) {
            paramVDG[igh++] =  *pimage++ & 0x00FF;
            paramVDG[igh++] =  *pimage++ & 0x00FF;
            iz++;

    		if (iz == 64 && ic < 8) {
    	        ic++;

                commVDG(paramVDG);    
            
                paramVDG[0] =  130;
                paramVDG[1] =  0xDE;
                paramVDG[2] =  ic;
                igh = 3;

                iz = 0;
    		}
    	}
        commVDG(paramVDG);    

        // Mostra a Imagem
        paramVDG[0] =  9;
        paramVDG[1] =  0xDF;
        paramVDG[2] =  x >> 8;
        paramVDG[3] =  x;
        paramVDG[4] =  y >> 8;
        paramVDG[5] =  y;
        paramVDG[6] =  ph >> 8;
        paramVDG[7] =  ph;
        paramVDG[8] =  pw >> 8;
        paramVDG[9] =  pw;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void startMGI(void) {
    unsigned char vnomefile[12];
    unsigned char lc, ll, *ptr_ico, *ptr_prg, *ptr_pos;
    unsigned char* vMemSystemAreaPos;

    #ifdef __USE_TFT_VDG__
        paramVDG[0] =  2;
        paramVDG[1] =  0xD8;
        paramVDG[2] =  0;
        commVDG(paramVDG);    

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
        voutput = 2;

        vcorwf = White;
        vcorwb = Blue;

        vparamstr[0] = '\0';
        vparam[0] = 20;
        vparam[1] = 80;
        vparam[2] = 280;
        vparam[3] = 100;
        vparam[4] = BTNONE;
        showWindow();

        writesxy(140,85,16,"MGI",vcorwf,vcorwb);
        writesxy(74,105,8,"Graphical Interface",vcorwf,vcorwb);
        writesxy(94,166,8,"Please Wait...",vcorwf,vcorwb);

        writesxy(86,155,8,"Loading Config",vcorwf,vcorwb);
        vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_MGICFG);
        _strcat(vnomefile,"MMSJMGI",".CFG");
        loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

        writesxy(86,155,8,"Loading Icons ",vcorwf,vcorwb);
        vMemSystemAreaPos = (unsigned char*)(vMemSystemArea + MEM_POS_ICONES);
        _strcat(vnomefile,"MOREICON",".LIB");
        loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

        redrawMain();

        while(editortela());

        voutput = 1;
        vcol = 0;
        vlin = 0;
        voverx = 0;
        vovery = 0;
        vxmaxold = 0;
        vymaxold = 0;

        clearScr(vcorb);

        paramVDG[0] = 2;
        paramVDG[1] = 0xD8;
        paramVDG[2] = 1;
        commVDG(paramVDG);    
    #endif
}

//-----------------------------------------------------------------------------
void redrawMain(void) {
    #ifdef __USE_TFT_VDG__
        clearScr(Blue);

        // Desenhar Barra Menu Principal / Status
        desenhaMenu();

        // Desenhar Icones da tela (lendo do disco)
        desenhaIconesUsuario();
    #endif
}

//-----------------------------------------------------------------------------
void desenhaMenu(void) {
    unsigned char lc;
    unsigned int vx, vy;

    #ifdef __USE_TFT_VDG__
        vx = COLMENU;
        vy = LINMENU;

        for (lc = 50; lc <= 56; lc++) {
            MostraIcone(vx, vy, lc);
            vx += 32;
        }

        FillRect(0, (vygmax - 35), vxgmax, 35, l_gray);
    #endif
}

//-----------------------------------------------------------------------------
void desenhaIconesUsuario(void) {
  unsigned int vx, vy;
  unsigned char lc, *ptr_ico, *ptr_prg, *ptr_pos;

  #ifdef __USE_TFT_VDG__
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
  #endif
}

//-----------------------------------------------------------------------------
void MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone) {
    unsigned char vnomefile[12];
    unsigned char *ptr_prg;
    unsigned char *ptr_viconef;

    #ifdef __USE_TFT_VDG__
        ptr_prg = (unsigned char*)(vMemSystemArea + (MEM_POS_MGICFG + 16) + 32 + 320);

        // Procura Icone no Disco se Nao for Padrao
        if (vicone < 50) {
            ptr_prg = ptr_prg + (vicone * 10);
            _strcat(vnomefile,(char*)ptr_prg,".ICO");
            loadFile(vnomefile, (unsigned char*)&mcfgfile);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
            if (verro)
                vicone = 59;
            else
                ptr_viconef = (unsigned char*)viconef;
        }

        if (vicone >= 50) {
            vicone -= 50;
            ptr_viconef = (unsigned char*)(vMemSystemArea + (MEM_POS_ICONES + (1152 * vicone)));
        }

        // Mostra Icone
        PutIcone(ptr_viconef, vvx, vvy);
    #endif
}

//--------------------------------------------------------------------------
unsigned char editortela(void) {
    unsigned char vresp = 1;
    unsigned char vx, cc, vpos, vposiconx, vposicony;
    unsigned int vbytepic;
    unsigned char *ptr_prg;

    #ifdef __USE_TFT_VDG__
        VerifyTouchLcd(WHAITTOUCH);

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
                InvertRect( vposiconx, vposicony, 24, 24);

                strcpy((char*)vbuf,(char *)ptr_prg);

                MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

                processCmd();

                *vbuf = 0x00;

                redrawMain();
              }
            }
        }
    #endif

    return vresp;
}

//-------------------------------------------------------------------------
unsigned char new_menu(void) {
    unsigned int vx, vy, lc, vposicony, mx, my, menyi[8], menyf[8];
    unsigned char vpos = 0, vresp, mpos;

    #ifdef __USE_TFT_VDG__
        vresp = 1;

        if (vpostx >= COLMENU && vpostx <= (COLMENU + 24)) {
            mx = 0;
            my = LINHAMENU;
            mpos = 0;

            FillRect(mx,my,128,42,White);
            DrawRect(mx,my,128,42,Black);

            mpos += 2;
            menyi[0] = my + mpos;
            writesxy(mx + 8,my + mpos,8,"Format",Black,White);
            mpos += 12;
            menyf[0] = my + mpos;
            DrawLine(mx,my + mpos,mx+128,my + mpos,Black);

            mpos += 2;
            menyi[1] = my + mpos;
            writesxy(mx + 8,my + mpos,8,"Help",Black,White);
            mpos += 12;
            menyf[1] = my + mpos;
            mpos += 2;
            menyi[2] = my + mpos;
            writesxy(mx + 8,my + mpos,8,"About",Black,White);
            mpos += 12;
            menyf[2] = my + mpos;
            DrawLine(mx,my + mpos,mx+128,my + mpos,Black);

            VerifyTouchLcd(WHAITTOUCH);

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
                    InvertRect( mx + 4, vposicony, 120, 12);

                switch (vpos) {
                    case 0: // Format
    /*                    strcpy(vbuf,"FORMAT\0");

                        MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

                        processCmd();

                        *vbuf = 0x00;*/
                        break;
                    case 1: // Help
                        break;
                    case 2: // About
                        message("MGI v0.1\nGraphical Interface\n \nwww.utilityinf.com.br\0", BTCLOSE, 0);
                        break;
                }
            }

            redrawMain();
        }
        else {
            for (lc = 1; lc <= 6; lc++) {
                mx = COLMENU + (32 * lc);
                if (vpostx >= mx && vpostx <= (mx + 24)) {
                    InvertRect( mx, 4, 24, 24);
                    InvertRect( mx, 4, 24, 24);
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
                    strcpy((char *)vbuf,"MDOS\0");

                    MostraIcone(144, 104, ICON_HOURGLASS);

                    processCmd();

                    *vbuf = 0x00;

                    break;
                case 5: // SETUP
                    break;
                case 6: // EXIT
                    mpos = message("Deseja Sair ?\0", BTYES | BTNO, 0);
                    if (mpos == BTYES)
                        vresp = 0;
                    else
                        redrawMain();

                    break;
            }

            if (lc < 6)
                redrawMain();
        }
    #endif

    return vresp;
}

//------------------------------------------------------------------------
void VerifyTouchLcd(unsigned char vtipo) {
    unsigned int vbytepic = 0;
    unsigned char vtec;
    char* sqtdtam;

    vbytepic = 0;

    vpostx = 0;
    vposty = 0;

    while (!vbytepic) {
        getKey();

        vbytepic = paramVDG[0];

        if (vbytepic == 0xFF) {
            paramVDG[0] =  0x01;
            paramVDG[1] =  0xDD;
            commVDG(paramVDG);    

            vposty = paramVDG[0] << 8;
            vposty += paramVDG[1] & 0x00FF;
            vpostx = paramVDG[2] << 8;
            vpostx += paramVDG[3] & 0x00FF;
        }

        if (!vtipo)
            break;
    }
}

//-------------------------------------------------------------------------
void new_icon(void) {
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
    _strcat(vnomefile,icon_prg[next_pos],".BIN");
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

      _strcat(vnomefile,"MGI",".CNF");
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
void del_icon(void) {
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

          _strcat(vnomefile,"MGI",".CNF");
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
void mgi_setup(void) {
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

    _strcat(vnomefile,"MGI",".CNF");
    vendmsb = 0xE5;
    vendlsb = 0x10;
    GravaArquivoMem(530);
  }

  RestoreScreen(4,3,121,60);
*/
}

//-------------------------------------------------------------------------
void executeCmd(void) {
    unsigned char vstring[64], vwb;

    #ifdef __USE_TFT_VDG__
        vstring[0] = '\0';

        strcpy((char *)vparamstr,"Execute");
        vparam[0] = 10;
        vparam[1] = 40;
        vparam[2] = 280;
        vparam[3] = 50;
        vparam[4] = BTOK | BTCANCEL;
        showWindow();

        writesxy(12,55,8,"Execute:",vcorwf,vcorwb);
        fillin(vstring, 84, 55, 160, WINDISP);

        while (1) {
            fillin(vstring, 84, 55, 160, WINOPER);

            vwb = waitButton();

            if (vwb == BTOK || vwb == BTCANCEL)
                break;
        }

        if (vwb == BTOK) {
            strcpy((char *)vbuf, (char *)vstring);

            MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

            // Chama processador de comandos
            processCmd();

            while (vxmaxold != 0) {
                vwb = waitButton();

                if (vwb == BTCLOSE)
                    break;
            }

            if (vxmaxold != 0) {
                vxmax = vxmaxold;
                vymax = vymaxold;
                vcol = 0;
                vlin = 0;
                voverx = 0;
                vovery = 0;
                vxmaxold = 0;
                vymaxold = 0;
            }

            *vbuf = 0x00;  // Zera Buffer do teclado
        }
    #endif
}

//-------------------------------------------------------------------------
unsigned char message(char* bstr, unsigned char bbutton, unsigned int btime)
{
	unsigned int i, ii = 0, iii, xi, yi, xf, xm, yf, ym, pwidth, pheight, xib, yib, xic, yic;
	unsigned char qtdnl, maxlenstr;
	unsigned char qtdcstr[8], poscstr[8], cc, dd, vbty = 0;
	unsigned char *bstrptr;

    #ifdef __USE_TFT_VDG__
    	qtdnl = 1;
    	maxlenstr = 0;
    	qtdcstr[1] = 0;
    	poscstr[1] = 0;
    	i = 0;

        for (ii = 0; ii <= 7; ii++)
            vbuttonwin[ii] = 0;

        bstrptr = bstr;
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

    	pwidth = maxlenstr * 8;
    	pwidth = pwidth + 2;
    	xm = pwidth / 2;
    	xi = 160 - xm - 1;
    	xf = 160 + xm - 1;

    	pheight = 10 * qtdnl;
    	pheight = pheight + 20;
    	ym = pheight / 2;
    	yi = 120 - ym - 1;
    	yf = 120 + ym - 1;

    	// Desenha Linha Fora
        SaveScreen(0x03,xi+2,yi,pwidth,pheight);

        FillRect(xi,yi,pwidth,pheight,White);
        vparam[0] = xi;
        vparam[1] = yi;
        vparam[2] = pwidth;
        vparam[3] = pheight;
        vparam[4] = 2;
        vparam[5] = Black;
    	DrawRoundRect();  // rounded rectangle around text area

    	// Escreve Texto Dentro da Caixa de Mensagem
    	for (i = 1; i <= qtdnl; i++)
    	{
    		xib = xi + xm;
    		xib = xib - ((qtdcstr[i] * 8) / 2);
    		yib = yi + 2 + (10 * (i - 1));

            locatexy(xib, yib);
            bstrptr = bstr + poscstr[i];
    		for (ii = poscstr[i]; ii <= (poscstr[i] + qtdcstr[i] - 1) ; ii++)
                writecxy(8, *bstrptr++, Black, White);
    	}

    	// Desenha Botoes
        i = 1;
        vbbutton = bbutton;
    	while (vbbutton)
    	{
    		xib = xi + 2 + (44 * (i - 1));
    		yib = yf - 12;
            vbty = yib;
    		i++;

            drawButtons(xib, yib);
    	}

      ii = 0;

      if (!btime) {
        while (!ii) {
      	  VerifyTouchLcd(WHAITTOUCH);

          for (i = 1; i <= 7; i++) {
            if (vbuttonwin[i] != 0 && vpostx >= vbuttonwin[i] && vpostx <= (vbuttonwin[i] + 32) && vposty >= vbty && vposty <= (vbty + 10)) {
              ii = 1;

              for (iii = 1; iii <= (i - 1); iii++)
                ii *= 2;

              break;
            }
          }
        }
      }
      else {
        for (dd = 0; dd <= 10; dd++)
          for (cc = 0; cc <= btime; cc++);
      }

      RestoreScreen(0x03, xi+2,yi,pwidth,pheight);
    #endif

    return ii;
}

//-------------------------------------------------------------------------
void showWindow(void)
{
	unsigned int i, ii, xib, yib, x1, y1, pwidth, pheight;
    unsigned char cc = 0, *bstr, bbutton;
    char* sqtdtam;

    #ifdef __USE_TFT_VDG__
        bstr = vparamstr;
        x1 = vparam[0];
        y1 = vparam[1];
        pwidth = vparam[2];
        pheight = vparam[3];
        bbutton = vparam[4];

        // Desenha a Janela
    	FillRect(x1, y1, pwidth, pheight, vcorwb);
        DrawRect(x1, y1, pwidth, pheight, vcorwf);

        if (*bstr) {
            DrawRect(x1, y1, pwidth, 12, vcorwf);
            writesxy(x1 + 2, y1 + 3,8,(char*)bstr,vcorwf,vcorwb);
        }

        i = 1;
        for (ii = 0; ii <= 7; ii++)
            vbuttonwin[ii] = 0;

    	// Desenha Botoes
        vbbutton = bbutton;
    	while (vbbutton)
    	{
    		xib = x1 + 2 + (44 * (i - 1));
    		yib = (y1 + pheight) - 12;
            vbuttonwiny = yib;
    		i++;

            drawButtons(xib, yib);
    	}
    #endif
}

//-------------------------------------------------------------------------
void drawButtons(unsigned int xib, unsigned int yib) {

    #ifdef __USE_TFT_VDG__
        // Desenha Bot?
        vparam[0] = xib;
        vparam[1] = yib;
        vparam[2] = 42;
        vparam[3] = 10;
        vparam[4] = 1;
        vparam[5] = Black;
    	FillRect(xib, yib, 42, 10, White);
    	DrawRoundRect();  // rounded rectangle around text area

    	// Escreve Texto do Bot?
    	if (vbbutton & BTOK)
    	{
    		writesxy(xib + 16 - 6, yib + 2,8,"OK",Black,White);
            vbbutton = vbbutton & 0xFE;    // 0b11111110
            vbuttonwin[1] = xib;
    	}
    	else if (vbbutton & BTSTART)
    	{
    		writesxy(xib + 16 - 15, yib + 2,8,"START",Black,White);
            vbbutton = vbbutton & 0xDF;    // 0b11011111
            vbuttonwin[6] = xib;
    	}
    	else if (vbbutton & BTCLOSE)
    	{
    		writesxy(xib + 16 - 15, yib + 2,8,"CLOSE",Black,White);
            vbbutton = vbbutton & 0xBF;    // 0b10111111
            vbuttonwin[7] = xib;
    	}
    	else if (vbbutton & BTCANCEL)
    	{
    		writesxy(xib + 16 - 12, yib + 2,8,"CANC",Black,White);
            vbbutton = vbbutton & 0xFD;    // 0b11111101
            vbuttonwin[2] = xib;
    	}
    	else if (vbbutton & BTYES)
    	{
    		writesxy(xib + 16 - 9, yib + 2,8,"YES",Black,White);
            vbbutton = vbbutton & 0xFB;    // 0b11111011
            vbuttonwin[3] = xib;
    	}
    	else if (vbbutton & BTNO)
    	{
    		writesxy(xib + 16 - 6, yib + 2,8,"NO",Black,White);
            vbbutton = vbbutton & 0xF7;    // 0b11110111
            vbuttonwin[4] = xib;
    	}
    	else if (vbbutton & BTHELP)
    	{
    		writesxy(xib + 16 - 12, yib + 2,8,"HELP",Black,White);
            vbbutton = vbbutton & 0xEF;    // 0b11101111
            vbuttonwin[5] = xib;
    	}
    #endif
}

//-------------------------------------------------------------------------
unsigned char waitButton(void) {
  unsigned char i, ii, iii;

  #ifdef __USE_TFT_VDG__
      ii = 0;
      VerifyTouchLcd(WHAITTOUCH);

      for (i = 1; i <= 7; i++) {
        if (vbuttonwin[i] != 0 && vpostx >= vbuttonwin[i] && vpostx <= (vbuttonwin[i] + 32) && vposty >= vbuttonwiny && vposty <= (vbuttonwiny + 10)) {
          ii = 1;

          for (iii = 1; iii <= (i - 1); iii++)
            ii *= 2;

          break;
        }
      }

      return ii;
  #endif      
}

//-------------------------------------------------------------------------
void fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo)
{
    unsigned int cc = 0;
    unsigned char cchar, *vvarptr, vdisp = 0;

    #ifdef __USE_TFT_VDG__
        vvarptr = vvar;

        while (*vvarptr) {
            cc += 8;
            vvarptr++;
        }

        if (vtipo == WINOPER) {
            if (!vkeyopen && vpostx >= x && vpostx <= (x + pwidth) && vposty >= y && vposty <= (y + 10)) {
                vkeyopen = 0x01;
                funcKey(1,1, 0, 0,x,y+12);
            }

            if (vbytetec == 0xFF) {
                if (vkeyopen && (vpostx < x || vpostx > (x + pwidth) || vposty < y || vposty > (y + 10))) {
                    vkeyopen = 0x00;
                    funcKey(1,2, 0, 0,x,y+12);
                }
            }
            else {
                if (vbytetec >= 0x20 && vbytetec <= 0x7F && (x + cc + 8) < (x + pwidth)) {
                    *vvarptr++ = vbytetec;
                    *vvarptr = 0x00;

                    locatexy(x+cc+2,y+2);
                    writecxy(8, vbytetec, Black, White);

                    vdisp = 1;
                }
                else {
                    switch (vbytetec) {
                        case 0x0D:  // Enter ou Tecla END
                            if (vkeyopen) {
                                vkeyopen = 0x00;
                                funcKey(1,2, 0, 0,x,y+12);
                            }
                            break;
                        case 0x08:  // BackSpace
                            if (pposx > (x + 10)) {
                                *vvarptr = '\0';
                                vvarptr--;
                                if (vvarptr < vvar)
                                    vvarptr = vvar;
                                *vvarptr = '\0';
                                pposx = pposx - 8;
                                locate(vcol,vlin, NOREPOS_CURSOR);
                                writecxy(8, 0x08, Black, White);
                                pposx = pposx - 8;
                            }
                            break;
                    }
                }
            }
        }

        if (vtipo == WINDISP || vdisp) {
            if (!vdisp) {
                DrawRect(x,y,pwidth,10,Black);
                FillRect(x+1,y+1,pwidth-2,8,White);
            }

            vvarptr = vvar;
            locatexy(x+2,y+2);
            while (*vvarptr) {
                cchar = *vvarptr++;
                cc++;

                writecxy(8, cchar, Black, White);

                if (pposx >= x + pwidth)
                    break;
            }
        }
    #endif
}

//-------------------------------------------------------------------------
void radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
  unsigned char cc, xc;
  unsigned char cchar, vdisp = 0;

  #ifdef __USE_TFT_VDG__
      xc = 0;
      cc = 0;
      cchar = ' ';

      while(vtipo == WINOPER && cchar != '\0') {
        cchar = vopt[cc];
        if (cchar == ',') {
          if (cchar == ',' && cc != 0)
            xc++;

          if (vpostx >= x && vpostx <= x + 8 && vposty >= (y + (xc * 10)) && vposty <= ((y + (xc * 10)) + 8)) {
            vvar[0] = xc;
            vdisp = 1;
          }
        }

        cc++;
      }

      xc = 0;
      cc = 0;

      while(vtipo == WINDISP || vdisp) {
        cchar = vopt[cc];

        if (cchar == ',') {
          if (cchar == ',' && cc != 0)
            xc++;

          FillRect(x, y + (xc * 10), 8, 8, White);
          DrawCircle(x + 4, y + (xc * 10) + 2, 4, 0, Black);

          if (vvar[0] == xc)
            DrawCircle(x + 4, y + (xc * 10) + 2, 3, 1, Black);
          else
            DrawCircle(x + 4, y + (xc * 10) + 2, 3, 0, Black);

          locatexy(x + 10, y + (xc * 10));
        }

        if (cchar != ',' && cchar != '\0')
          writecxy(8, cchar, Black, White);

        if (cchar == '\0')
          break;

        cc++;
      }
  #endif
}

//-------------------------------------------------------------------------
void togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
  unsigned char cc = 0;
  unsigned char cchar, vdisp = 0;

  #ifdef __USE_TFT_VDG__
      if (vtipo == WINOPER && vpostx >= x && vpostx <= x + 4 && vposty >= y && vposty <= y + 4) {
        if (vvar[0])
          vvar[0] = 0;
        else
          vvar[0] = 1;

        vdisp = 1;
      }

      if (vtipo == WINDISP || vdisp) {
        FillRect(x, y + 2, 4, 4, White);
        DrawRect(x, y + 2, 4, 4, Black);

        if (vvar[0]) {
          DrawLine(x, y + 2, x + 4, y + 6, Black);
          DrawLine(x, y + 6, x + 4, y + 2, Black);
        }  

        if (vtipo == WINDISP) {
          x += 6;
          locatexy(x,y);
          while (bstr[cc] != 0) {
            cchar = bstr[cc];
            cc++;

            writecxy(8, cchar, Black, White);
            x += 6;
          }
        }
      }
  #endif
}


//-------------------------------------------------------------------------
/*void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/

//-------------------------------------------------------------------------
/*void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/

//-------------------------------------------------------------------------
void runCmd(void)
{

}
