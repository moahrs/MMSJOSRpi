#include "kernel/mmsjos_api.h"
#include <drivers/lcd_vdg_api.h>

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
unsigned char  next_pos;

unsigned char diskDrv;
unsigned long vclusterdir;
unsigned long vclusteros;
unsigned char  *gDataBuffer; // The global data sector buffer
unsigned char  *mcfgfile; // onde eh carregado o arquivo de configuracao e outros arquivos 12K

unsigned int  viconef; // onde eh carregado o arquivo de configuracao e outros arquivos 12K
unsigned int  vcorf; // cor padrao de frente
unsigned int  vcorb; // cor padrao de fundo
unsigned int  vcol;
unsigned int  vlin;
unsigned int  voutput; // 0 - LCD (16x2), 1 - LCD Grafico (320 x 240), 2 - VGA (somente modo texto)
unsigned char  *vbuf; // Buffer Linha Digitavel OS 32 bytes
unsigned int  vxmax;
unsigned int  vymax;
unsigned int  xpos;
unsigned int  ypos;
unsigned int  verro;
unsigned char  *vdiratu; // Buffer de pasta atual 128 bytes
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


//-----------------------------------------------------------------------------
unsigned char * _strcat (unsigned char * dst, char * cp, char * src) {
    while( *cp )
        *dst++ = *cp++;     /* copy to dst and find end of dst */

    while( *src )
        *dst++ = *src++;       /* Copy src to end of dst */

    *dst++ = 0x00;

    return( dst );                  /* return dst */
}

//-------------------------------------------------------------------------
unsigned int bcd2dec(unsigned int bcd)
{
    unsigned int dec=0;
    unsigned int mult;
    for (mult=1; bcd; bcd=bcd>>4,mult*=10)
        dec += (bcd & 0x0f) * mult;
    return dec;
}

//-------------------------------------------------------------------------
unsigned int datetimetodir(unsigned char hr_day, unsigned char min_month, unsigned char sec_year, unsigned char vtype)
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

/*
//-----------------------------------------------------------------------------//-----------------------------------------------------------------------------
void clearScr(unsigned int pcolor) {
    paramVDG[0] = 2;
    paramVDG[1] = 0xD0;
    paramVDG[2] = pcolor;
    commVDG(paramVDG);

    locateScr(0, 0, REPOS_CURSOR);
}

//-----------------------------------------------------------------------------
void printPromptScr(unsigned int plinadd) {
    if (plinadd)
        *vlin = *vlin + 1;

    locateScr(0,*vlin, NOREPOS_CURSOR);

    printStrScr("#\0", White, Black);
    printStrScr(vdiratu, White, Black);
    printStrScr(">\0", White, Black);

    *vinip = *vcol;
}

//-----------------------------------------------------------------------------
void printStrScr(unsigned char *msgs, unsigned int pcolor, unsigned int pbcolor) {
    unsigned char ix = 10, iy, ichange = 0;
    unsigned char *ss = msgs;

    while (*ss) {
      if (*ss >= 0x20)
          ix++;
      else
          ichange = 1;

      if ((*vcol + (ix - 10)) > *vxmax)
          ichange = 2;

      *ss++;

      if (!*ss && !ichange)
         ichange = 3;

      if (ichange) {
         // Manda Sequencia de Controle
         if (ix > 10) {
            paramVDG[0] = ix;
            paramVDG[1] = 0xD1;
            paramVDG[2] = (*vcol * 8) >> 8;
            paramVDG[3] = *vcol * 8;
            paramVDG[4] = (*vlin * 10) >> 8;
            paramVDG[5] = *vlin * 10;
            paramVDG[6] = 8;
            paramVDG[7] = pcolor >> 8;
            paramVDG[8] = pcolor;
            paramVDG[9] = pbcolor >> 8;
            paramVDG[10] = pbcolor;
         }

         if (ichange == 1)
            ix++;

         iy = 11;
         while (*msgs && iy <= ix) {
            if (*msgs >= 0x20) {
                paramVDG[iy] = *msgs;
                *vcol = *vcol + 1;
            }
            else {
                if (*msgs == 0x0D) {
                    *vcol = 0;
                }
                else if (*msgs == 0x0A) {
                    *vcol = 0;  // So para teste, despois tiro e coloco '\r' junto com '\n'
                    *vlin = *vlin + 1;
                }

                locateScr(*vcol, *vlin, NOREPOS_CURSOR);
            }

            *msgs++;
            iy++;
        }
        commVDG(paramVDG);

        if (ichange == 2) {
            *vcol = 0;
            *vlin = *vlin + 1;
            locateScr(*vcol, *vlin, NOREPOS_CURSOR);
        }

        ichange = 0;
        ix = 10;
      }
    }
}

//-----------------------------------------------------------------------------
void printByteScr(unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor) {
    paramVDG[0] = 0x0B;
    paramVDG[1] = 0xD2;
    paramVDG[2] = (*vcol * 8) >> 8;
    paramVDG[3] = *vcol * 8;
    paramVDG[4] = (*vlin * 10) >> 8;
    paramVDG[5] = *vlin * 10;
    paramVDG[6] = 8;
    paramVDG[7] = pcolor >> 8;
    paramVDG[8] = pcolor;
    paramVDG[9] = pbcolor >> 8;
    paramVDG[10] = pbcolor;
    paramVDG[11] = pbyte;
    commVDG(paramVDG);

    *vcol = *vcol + 1;

    locateScr(*vcol, *vlin, REPOS_CURSOR_ON_CHANGE);
}

//-----------------------------------------------------------------------------
void locateScr(unsigned char pcol, unsigned char plin, unsigned char pcur) {
    unsigned int vend, ix, iy, ichange = 0;
    unsigned int vlcdf[16];

    if (pcol > *vxmax) {
        pcol = 0;
        plin++;
        ichange = 1;
    }

    if (plin > *vymax) {
        paramVDG[0] = 2;
        paramVDG[1] = 0xD9;
        paramVDG[2] = 10;
        commVDG(paramVDG);

        pcol = 0;
        plin = *vymax;
        ichange = 1;
    }

    *vcol = pcol;
    *vlin = plin;

    if (pcur == 1 || (pcur == 2 && ichange)) {
        printByteScr(0x08, White, Black);
        *vcol = *vcol - 1;
    }
}

//-----------------------------------------------------------------------------
void loadFile(unsigned short* xaddress)
{
  unsigned short cc, dd;
  unsigned short vrecfim, vbytepic, vbyteprog[128];
  unsigned int vbytegrava = 0;
  unsigned short xdado = 0, xcounter = 0;
  unsigned short vcrc, vcrcpic, vloop;

  vrecfim = 1;
  *verro = 0;

  while (vrecfim) {
    vloop = 1;
    while (vloop) {
        // Processa Retorno do PIC
      	recPic();

        if (vbytepic == picCommData) {
            // Carrega Dados Recebidos
            vcrc = 0;
    		for (cc = 0; cc <= 127 ; cc++)
      		{
          		recPic(); // Ler dados do PIC
      			vbyteprog[cc] = vbytepic;
      			vcrc += vbytepic;
      		}

            // Recebe 2 Bytes CRC
      		recPic();
      		vcrcpic = vbytepic;
      		recPic();
      		vcrcpic |= ((vbytepic << 8) & 0xFF00);

            if (vcrc == vcrcpic) {
                sendPic(0x01);
                sendPic(0xC5);
                vloop = 0;
            }
            else {
                sendPic(0x01);
                sendPic(0xFF);
            }
        }
        else if (vbytepic == picCommStop) {
            // Finaliza Comunicação Serial
            vloop = 0;
      		vrecfim = 0;
        }
        else {
            vloop = 0;
            vrecfim = 0;
            *verro = 1;
        }
    }

    if (vrecfim) {
        for (dd = 00; dd <= 127; dd += 2){
        	vbytegrava = vbyteprog[dd] << 8;
        	vbytegrava = vbytegrava | (vbyteprog[dd + 1] & 0x00FF);

            // Grava Dados na Posição Especificada
            *xaddress = vbytegrava;
            xaddress += 1;
        }
    }
  }
}
*/