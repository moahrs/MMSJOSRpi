/********************************************************************************
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

#include <circlelib/circlelib.h>
#include <stddef.h>
#include <stdint.h>
#include <drivers/uart.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include <drivers/bcm2835min.h>
#include "common/mylib.h"

CMGI *CMGI::s_pThis = 0;

CMGI::CMGI (CMMSJOS *mMMSJOS, CLcdVdg *mLcdVdg)
: p_mLcdVdg (mLcdVdg),
  p_mMMSJOS (mMMSJOS)
{
    s_pThis = this;
}

CMGI::~CMGI (void)
{
    s_pThis = 0;
    p_mLcdVdg = 0;
    p_mMMSJOS = 0;
}

boolean CMGI::Initialize (void)
{
    // Recuperar informacoes do Video
    paramVDG[0] = 0x01;
    paramVDG[1] = 0xEF;
    p_mLcdVdg->commVDG(paramVDG);

    vbytevdg = paramVDG[0];
    vxgmax = paramVDG[1] << 8;
    vxgmax = vxgmax | paramVDG[2];
    vygmax = paramVDG[3] << 8;
    vygmax = vygmax | paramVDG[4];

    if (vxgmax != 319 || vygmax != 239) {
        vxgmax = 319;
        vygmax = 239;
    }

    return TRUE;
}

void CMGI::writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor) {
    unsigned char ix = 10, *ss = (unsigned char*)msgs;
    unsigned int iy;

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

        p_mLcdVdg->commVDG(paramVDG);
    }
}

//-----------------------------------------------------------------------------
void CMGI::writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor) {
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

    p_mLcdVdg->commVDG(paramVDG);

    pposx = pposx + sizef;

    if ((pposx + sizef) > vxgmax)
        pposx = pposx - sizef;
}

//-----------------------------------------------------------------------------
void CMGI::locatexy(unsigned int xx, unsigned int yy) {
    pposx = xx;
    pposy = yy;
}

//-----------------------------------------------------------------------------
void CMGI::SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::SetDot(unsigned int x, unsigned int y, unsigned int color) {
    paramVDG[0] =  8;
    paramVDG[1] =  0xD7;
    paramVDG[2] =  x >> 8;
    paramVDG[3] =  x;
    paramVDG[4] =  y >> 8;
    paramVDG[5] =  y;
    paramVDG[6] =  0;
    paramVDG[7] =  color >> 8;
    paramVDG[8] =  color;
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor) {
    unsigned int xf, yf;

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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::DrawRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int color) {
    unsigned int xf, yf;

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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::DrawRoundRect(void) {
    unsigned int xi, yi, pwidth, pheight, color;
    unsigned char radius;
    int tSwitch;
    unsigned int x1 = 0, y1, xt, yt, wt;

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
    DrawHoriLine(xt, yi, wt, color);        // top
    DrawHoriLine(xt, yt, wt, color);    // bottom

    xt = xi + pwidth;
    yt = yi + radius;
    wt = pheight - (2 * radius);
    DrawVertLine(xi, yt, wt, color);        // left
    DrawVertLine(xt, yt, wt, color);    // right
}

//-----------------------------------------------------------------------------
void CMGI::DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor) {
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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    DrawRect((x - 1), (y - 1), (pwidth + 2), (pheight + 2), Red);
}

//-----------------------------------------------------------------------------
void CMGI::PutImage(unsigned char* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    (void)(vimage);
    (void)(x);
    (void)(y);
    (void)(pwidth);
    (void)(pheight);

    /* ver como fazer */
}

//-----------------------------------------------------------------------------
void CMGI::PutIcone(unsigned char* vimage, unsigned int x, unsigned int y) {
    unsigned int ix, iz, pw, ph;
    unsigned char* pimage;
    unsigned char ic, igh;

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

            p_mLcdVdg->commVDG(paramVDG);    
        
            paramVDG[0] =  130;
            paramVDG[1] =  0xDE;
            paramVDG[2] =  ic;
            igh = 3;

            iz = 0;
        }
    }
    p_mLcdVdg->commVDG(paramVDG);    

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
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::startMGI(void) {
    unsigned char vnomefile[12];
    unsigned char lc, ll, *ptr_ico, *ptr_prg, *ptr_pos;
    unsigned char* vMemSystemAreaPos;

    paramVDG[0] =  2;
    paramVDG[1] =  0xD8;
    paramVDG[2] =  0;
    p_mLcdVdg->commVDG(paramVDG);    

    ptr_pos = (unsigned char*)(p_mMMSJOS->vMemSystemArea + (MEM_POS_MGICFG + 16));
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
    p_mMMSJOS->voutput = 2;

    vcorwf = White;
    vcorwb = Blue;

    vparamstr[0] = '\0';
    vparam[0] = 20;
    vparam[1] = 80;
    vparam[2] = 280;
    vparam[3] = 100;
    vparam[4] = BTNONE;
    showWindow();

    writesxy(140,85,16,(char*)"MGI",vcorwf,vcorwb);
    writesxy(74,105,8,(char*)"Graphical Interface",vcorwf,vcorwb);
    writesxy(94,166,8,(char*)"Please Wait...",vcorwf,vcorwb);

    writesxy(86,155,8,(char*)"Loading Config",vcorwf,vcorwb);
    vMemSystemAreaPos = (unsigned char*)(p_mMMSJOS->vMemSystemArea + MEM_POS_MGICFG);
    p_mMMSJOS->_strcat(vnomefile,(char*)"MMSJMGI",(char*)".CFG");
    p_mMMSJOS->loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

    writesxy(86,155,8,(char*)"Loading Icons ",vcorwf,vcorwb);
    vMemSystemAreaPos = (unsigned char*)(p_mMMSJOS->vMemSystemArea + MEM_POS_ICONES);
    p_mMMSJOS->_strcat(vnomefile,(char*)"MOREICON",(char*)".LIB");
    p_mMMSJOS->loadFile(vnomefile, (unsigned char*)vMemSystemAreaPos);

    redrawMain();

    while(editortela());

    p_mMMSJOS->voutput = 1;
    p_mMMSJOS->vcol = 0;
    p_mMMSJOS->vlin = 0;
    p_mMMSJOS->voverx = 0;
    p_mMMSJOS->vovery = 0;
    p_mMMSJOS->vxmaxold = 0;
    p_mMMSJOS->vymaxold = 0;

    p_mMMSJOS->clearScr(p_mMMSJOS->vcorb);

    paramVDG[0] = 2;
    paramVDG[1] = 0xD8;
    paramVDG[2] = 1;
    p_mLcdVdg->commVDG(paramVDG);    
}

//-----------------------------------------------------------------------------
void CMGI::redrawMain(void) {
    p_mMMSJOS->clearScr(Blue);

    // Desenhar Barra Menu Principal / Status
    desenhaMenu();

    // Desenhar Icones da tela (lendo do disco)
    desenhaIconesUsuario();
}

//-----------------------------------------------------------------------------
void CMGI::desenhaMenu(void) {
    unsigned char lc;
    unsigned int vx, vy;

    vx = COLMENU;
    vy = LINMENU;

    for (lc = 50; lc <= 56; lc++) {
        MostraIcone(vx, vy, lc);
        vx += 32;
    }

    FillRect(0, (vygmax - 35), vxgmax, 35, l_gray);
}

//-----------------------------------------------------------------------------
void CMGI::desenhaIconesUsuario(void) {
  unsigned int vx, vy;
  unsigned char lc, *ptr_ico, *ptr_prg, *ptr_pos;

  // COLOCAR ICONSPERLINE = 10
  // COLOCAR SPACEICONS = 8
  
  next_pos = 0;

  ptr_pos = (unsigned char*)(p_mMMSJOS->vMemSystemArea + (MEM_POS_MGICFG + 16));
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
void CMGI::MostraIcone(unsigned int vvx, unsigned int vvy, unsigned char vicone) {
    unsigned char vnomefile[12];
    unsigned char *ptr_prg;
    unsigned char *ptr_viconef;

    ptr_prg = (unsigned char*)(p_mMMSJOS->vMemSystemArea + (MEM_POS_MGICFG + 16) + 32 + 320);

    // Procura Icone no Disco se Nao for Padrao
    if (vicone < 50) {
        ptr_prg = ptr_prg + (vicone * 10);
        p_mMMSJOS->_strcat(vnomefile,(char*)ptr_prg,(char*)".ICO");
        p_mMMSJOS->loadFile(vnomefile, (unsigned char*)&p_mMMSJOS->mcfgfile);   // 12K espaco pra carregar arquivo. Colocar logica pra pegar tamanho e alocar espaco
        if (verro)
            vicone = 59;
        else
            ptr_viconef = (unsigned char*)viconef;
    }

    if (vicone >= 50) {
        vicone -= 50;
        ptr_viconef = (unsigned char*)(p_mMMSJOS->vMemSystemArea + (MEM_POS_ICONES + (1152 * vicone)));
    }

    // Mostra Icone
    PutIcone(ptr_viconef, vvx, vvy);
}

//--------------------------------------------------------------------------
unsigned char CMGI::editortela(void) {
    unsigned char vresp = 1;
    unsigned char vx, cc, vpos, vposiconx, vposicony;
    unsigned char *ptr_prg;

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

          ptr_prg = (unsigned char*)(p_mMMSJOS->vMemSystemArea + (MEM_POS_MGICFG + 16) + 32 + 320);
          ptr_prg = ptr_prg + (vpos * 10);

          if (*ptr_prg != 0) {
            InvertRect( vposiconx, vposicony, 24, 24);

            strcpy((char*)p_mMMSJOS->vbuf,(char *)ptr_prg);

            MostraIcone(144, 104, ICON_HOURGLASS);  // Mostra Ampulheta

            p_mMMSJOS->processCmd();

            *p_mMMSJOS->vbuf = 0x00;

            redrawMain();
          }
        }
    }

    return vresp;
}

//-------------------------------------------------------------------------
unsigned char CMGI::new_menu(void) {
    unsigned int vy, lc, vposicony, mx, my, menyi[8], menyf[8];
    unsigned char vpos = 0, vresp, mpos;

    vresp = 1;

    if (vpostx >= COLMENU && vpostx <= (COLMENU + 24)) {
        mx = 0;
        my = LINHAMENU;
        mpos = 0;

        FillRect(mx,my,128,42,White);
        DrawRect(mx,my,128,42,Black);

        mpos += 2;
        menyi[0] = my + mpos;
        writesxy(mx + 8,my + mpos,8,(char*)"Format",Black,White);
        mpos += 12;
        menyf[0] = my + mpos;
        DrawLine(mx,my + mpos,mx+128,my + mpos,Black);

        mpos += 2;
        menyi[1] = my + mpos;
        writesxy(mx + 8,my + mpos,8,(char*)"Help",Black,White);
        mpos += 12;
        menyf[1] = my + mpos;
        mpos += 2;
        menyi[2] = my + mpos;
        writesxy(mx + 8,my + mpos,8,(char*)"About",Black,White);
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
                    message((char*)"MGI v0.1\nGraphical Interface\n \nwww.utilityinf.com.br\0", BTCLOSE, 0);
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
                strcpy((char *)p_mMMSJOS->vbuf,(char*)"MDOS\0");

                MostraIcone(144, 104, ICON_HOURGLASS);

                p_mMMSJOS->processCmd();

                *p_mMMSJOS->vbuf = 0x00;

                break;
            case 5: // SETUP
                break;
            case 6: // EXIT
                mpos = message((char*)"Deseja Sair ?\0", BTYES | BTNO, 0);
                if (mpos == BTYES)
                    vresp = 0;
                else
                    redrawMain();

                break;
        }

        if (lc < 6)
            redrawMain();
    }

    return vresp;
}

//------------------------------------------------------------------------
void CMGI::VerifyTouchLcd(unsigned char vtipo) {
    unsigned int vbytepic = 0;

    vbytepic = 0;

    vpostx = 0;
    vposty = 0;

    while (!vbytepic) {
        getKey();

        vbytepic = paramVDG[0];

        if (vbytepic == 0xFF) {
            paramVDG[0] =  0x01;
            paramVDG[1] =  0xDD;
            p_mLcdVdg->commVDG(paramVDG);    

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
void CMGI::new_icon(void) {
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
void CMGI::del_icon(void) {
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
void CMGI::mgi_setup(void) {
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

//-------------------------------------------------------------------------
void CMGI::executeCmd(void) {
    unsigned char vstring[64], vwb;

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
    }
}

//-------------------------------------------------------------------------
unsigned char CMGI::message(char* bstr, unsigned char bbutton, unsigned int btime)
{
    unsigned int i, ii = 0, iii, xi, yi, xm, yf, ym, pwidth, pheight, xib, yib;
    unsigned char qtdnl, maxlenstr;
    unsigned char qtdcstr[8], poscstr[8], cc, dd, vbty = 0;
    unsigned char *bstrptr;

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

    pwidth = maxlenstr * 8;
    pwidth = pwidth + 2;
    xm = pwidth / 2;
    xi = 160 - xm - 1;

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
        bstrptr = (unsigned char*)bstr + poscstr[i];
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

    return ii;
}

//-------------------------------------------------------------------------
void CMGI::showWindow(void)
{
	unsigned int i, ii, xib, yib, x1, y1, pwidth, pheight;
    unsigned char *bstr, bbutton;

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
}

//-------------------------------------------------------------------------
void CMGI::drawButtons(unsigned int xib, unsigned int yib) {

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
        writesxy(xib + 16 - 6, yib + 2,8,(char*)"OK",Black,White);
        vbbutton = vbbutton & 0xFE;    // 0b11111110
        vbuttonwin[1] = xib;
    }
    else if (vbbutton & BTSTART)
    {
        writesxy(xib + 16 - 15, yib + 2,8,(char*)"START",Black,White);
        vbbutton = vbbutton & 0xDF;    // 0b11011111
        vbuttonwin[6] = xib;
    }
    else if (vbbutton & BTCLOSE)
    {
        writesxy(xib + 16 - 15, yib + 2,8,(char*)"CLOSE",Black,White);
        vbbutton = vbbutton & 0xBF;    // 0b10111111
        vbuttonwin[7] = xib;
    }
    else if (vbbutton & BTCANCEL)
    {
        writesxy(xib + 16 - 12, yib + 2,8,(char*)"CANC",Black,White);
        vbbutton = vbbutton & 0xFD;    // 0b11111101
        vbuttonwin[2] = xib;
    }
    else if (vbbutton & BTYES)
    {
        writesxy(xib + 16 - 9, yib + 2,8,(char*)"YES",Black,White);
        vbbutton = vbbutton & 0xFB;    // 0b11111011
        vbuttonwin[3] = xib;
    }
    else if (vbbutton & BTNO)
    {
        writesxy(xib + 16 - 6, yib + 2,8,(char*)"NO",Black,White);
        vbbutton = vbbutton & 0xF7;    // 0b11110111
        vbuttonwin[4] = xib;
    }
    else if (vbbutton & BTHELP)
    {
        writesxy(xib + 16 - 12, yib + 2,8,(char*)"HELP",Black,White);
        vbbutton = vbbutton & 0xEF;    // 0b11101111
        vbuttonwin[5] = xib;
    }
}

//-------------------------------------------------------------------------
unsigned char CMGI::waitButton(void) {
  unsigned char i, ii, iii;

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
}

//-------------------------------------------------------------------------
void CMGI::fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo)
{
    unsigned int cc = 0;
    unsigned char cchar, *vvarptr, vdisp = 0;

    vvarptr = vvar;

    while (*vvarptr) {
        cc += 8;
        vvarptr++;
    }

    if (vtipo == WINOPER) {
        if (!vkeyopen && vpostx >= x && vpostx <= (x + pwidth) && vposty >= y && vposty <= (y + 10)) {
            vkeyopen = 0x01;
            p_mMMSJOS->funcKey(1,1, 0, 0,x,y+12);
        }

        if (vbytetec == 0xFF) {
            if (vkeyopen && (vpostx < x || vpostx > (x + pwidth) || vposty < y || vposty > (y + 10))) {
                vkeyopen = 0x00;
                p_mMMSJOS->funcKey(1,2, 0, 0,x,y+12);
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
                            p_mMMSJOS->funcKey(1,2, 0, 0,x,y+12);
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
                            p_mMMSJOS->locate(p_mMMSJOS->vcol,p_mMMSJOS->vlin, NOREPOS_CURSOR);
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
}

//-------------------------------------------------------------------------
void CMGI::radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
  unsigned char cc, xc;
  unsigned char cchar, vdisp = 0;

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
}

//-------------------------------------------------------------------------
void CMGI::togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
  unsigned char cc = 0;
  unsigned char cchar, vdisp = 0;

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
}

//-------------------------------------------------------------------------
/*void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/

//-------------------------------------------------------------------------
/*void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/

//-------------------------------------------------------------------------
