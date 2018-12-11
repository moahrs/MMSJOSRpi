#include <circle/string.h>
#include <circle/types.h>
#include <circle/interrupt.h>
#include <ScreenTFT/screentft.h>
#include <circle/devicenameservice.h>
#include <circle/sysconfig.h>
#include <circle/util.h>
#include <stddef.h>
#include <stdint.h>

#define __USE_TFT_LCD__

CScrTft *CScrTft::s_pThis = 0;

CScrTft::CScrTft (CInterruptSystem *mInterrupt)
: m_pInterruptSystem (mInterrupt)
{
    s_pThis = this;
}

CScrTft::~CScrTft (void)
{
    s_pThis = 0;
    p_mLcdVdg = 0;
}

boolean CScrTft::Initialize (void)
{
    p_mLcdVdg = new CLcdVdg();
    p_mLcdVdg->Initialize();

    p_mLcdTch = new CLcdTch(m_pInterruptSystem, p_mLcdVdg);
    p_mLcdTch->Initialize();

    voutput = 0x01; // Indica Padrão Inicial Video TEXTO = 1, MGI = 2
    voverx = 0;   // Indica sem overlay, usando tela de texto.
    vovery = 0;   // Indica sem overlay, usando tela de texto.
    vxmaxold = 0;
    vymaxold = 0;

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

    vxmax = ((vxgmax + 1) / 8) - 1;
    vymax = ((vygmax + 1) / 10) - 1;
    vlin = 0;
    vcol = 0;
    vcorf = White;
    vcorb = Black;

    CDeviceNameService::Get ()->AddDevice ("ScrLdg", this, FALSE);

    return true;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetWidth (void) const
{
    return vxgmax;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetHeight (void) const
{
    return vygmax;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetColumns (void) const
{
    return vxmax;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetRows (void) const
{
    return vymax;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetRow (void) const
{
    return vlin;
}

//-----------------------------------------------------------------------------
void CScrTft::SetRow (unsigned pRow)
{
    vlin = pRow;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetColumn (void) const
{
    return vcol;
}

//-----------------------------------------------------------------------------
void CScrTft::SetColumn (unsigned pCol)
{
    vcol = pCol;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetColorForeground (void) const
{
    if (voutput == 1)
        return vcorf;
    else
        return vcorwf;
}

//-----------------------------------------------------------------------------
void CScrTft::SetColorForeground (unsigned pColorF)
{
    if (voutput == 1)
        vcorf = pColorF;
    else
        vcorwf = pColorF;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetColorBackground (void) const
{
    if (voutput == 1)
        return vcorb;
    else
        return vcorwb;
}

//-----------------------------------------------------------------------------
void CScrTft::SetColorBackground (unsigned pColorB)
{
    if (voutput == 1)
        vcorb = pColorB;
    else
        vcorwb = pColorB;
}

//-----------------------------------------------------------------------------
void CScrTft::clearScr() 
{
    clearScr(vcorb);
}

//-----------------------------------------------------------------------------
void CScrTft::clearScr(unsigned int pcolor) 
{
    paramVDG[0] = 3;
    paramVDG[1] = 0xD0;
    paramVDG[2] = pcolor >> 8;
    paramVDG[3] = pcolor;
    p_mLcdVdg->commVDG(paramVDG);

    locate(0, 0, REPOS_CURSOR);
}

//-----------------------------------------------------------------------------
int CScrTft::Write (const void *pBuffer, unsigned nCount)
{
    int nResult = 0;
    writestr((char*)pBuffer);
    nResult = nCount;
    return nResult;
}

//-----------------------------------------------------------------------------
void CScrTft::writechar(int c)
{
    writec((unsigned char)c, vcorf, vcorb, ADD_POS_SCR);
}

//-----------------------------------------------------------------------------
void CScrTft::writestr(char *msgs)
{
    writes(msgs, vcorf, vcorb);
}

//-----------------------------------------------------------------------------
void CScrTft::writes(char *msgs, unsigned int pcolor, unsigned int pbcolor) 
{
    unsigned char ix = 10, iy, ichange = 0;
    unsigned char *ss = (unsigned char*)msgs;
    unsigned int xcolor = pcolor, xbcolor = pbcolor;

    if (voutput == 2) {
        xcolor = vcorwf;
        xbcolor = vcorwb;
    }

    while (*ss) 
    {
        if (*ss >= 0x20)
            ix++;
        else
            ichange = 1;

        if ((vcol + (ix - 10)) > vxmax)
            ichange = 2;

        ss++;

        if (!*ss && !ichange)
            ichange = 3;

        if (ichange) 
        {
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
                }

                msgs++;
                iy++;
            }

            p_mLcdVdg->commVDG(paramVDG);

            if (vcol <= vxmax )
                locate(vcol, vlin, NOREPOS_CURSOR);

            if (ichange == 2)
            {
                 vcol = 0;
                 vlin++;
                 locate(vcol, vlin, NOREPOS_CURSOR);
            }

            ichange = 0;
            ix = 10;
        }
    }
}

//-----------------------------------------------------------------------------
void CScrTft::writec(unsigned char pbyte, unsigned char ptipo) 
{
    writec(pbyte, vcorf, vcorb, ptipo);
}

//-----------------------------------------------------------------------------
void CScrTft::writec(unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor, unsigned char ptipo) 
{
    unsigned int xcolor = pcolor, xbcolor = pbcolor;

    desativaCursor();

    if (voutput == 2) {
        xcolor = vcorwf;
        xbcolor = vcorwb;
    }

    if (pbyte != '\n' && pbyte != '\r')
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
        p_mLcdVdg->commVDG(paramVDG);

        if (ptipo == ADD_POS_SCR) {
            vcol = vcol + 1;

            if ((vlin == (vymax - 1)) && (vcol == vxmax)) {
                vcol = 0;
                vlin++;
            }

            locate(vcol, vlin, REPOS_CURSOR_ON_CHANGE);
        }
    }
    else if (pbyte == '\r')
    {
        vcol = 0;

        locate(vcol, vlin, REPOS_CURSOR_ON_CHANGE);
    }
    else if (pbyte == '\n')
    {
        vlin++;
        vcol = 0;

        locate(vcol, vlin, REPOS_CURSOR_ON_CHANGE);
    }

    ativaCursor();
}

//-----------------------------------------------------------------------------
void CScrTft::locate(unsigned char pcol, unsigned char plin, unsigned char pcur) 
{
    unsigned int ichange = 0;

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
            p_mLcdVdg->commVDG(paramVDG);
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
}

//-----------------------------------------------------------------------------
void CScrTft::funcKey(unsigned char vambiente, unsigned char vshow, unsigned char venter, unsigned char vtipo, unsigned int x, unsigned int y) {
    unsigned int vkeyx, vkeyy;

    // Calcula posicao do teclado
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

    p_mLcdVdg->commVDG(paramVDG);
}

//-----------------------------------------------------------------------------
void CScrTft::ativaCursor(void)
{ 
    paramVDG[0] = 0x02; 
    paramVDG[1] = 0xD8; 
    paramVDG[2] = 1; 
    p_mLcdVdg->commVDG(paramVDG); 
} 

//-----------------------------------------------------------------------------
void CScrTft::desativaCursor(void)
{ 
    paramVDG[0] = 0x02; 
    paramVDG[1] = 0xD8; 
    paramVDG[2] = 0; 
    p_mLcdVdg->commVDG(paramVDG); 
} 

//-----------------------------------------------------------------------------
void CScrTft::blinkCursor(void)
{ 
    paramVDG[0] = 0x01; 
    paramVDG[1] = 0xF0; 
    p_mLcdVdg->commVDG(paramVDG); 
} 

//-----------------------------------------------------------------------------
unsigned char CScrTft::verifKey(void)
{ 
    paramVDG[0] = 0x01; 
    paramVDG[1] = 0xED; 
    p_mLcdVdg->commVDG(paramVDG);

    return paramVDG[0];
}

//-----------------------------------------------------------------------------
unsigned char CScrTft::getKey(void)
{ 
    paramVDG[0] = 0x01; 
    paramVDG[1] = 0xDC; 
    p_mLcdVdg->commVDG(paramVDG); 

    return paramVDG[0];
}

//-----------------------------------------------------------------------------
void CScrTft::writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor) {
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
void CScrTft::writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor) {
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
void CScrTft::locatexy(unsigned int xx, unsigned int yy) {
    pposx = xx;
    pposy = yy;
}

//-----------------------------------------------------------------------------
void CScrTft::SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
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
void CScrTft::RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pPage) {
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
void CScrTft::SetDot(unsigned int x, unsigned int y, unsigned int color) {
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
void CScrTft::FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor) {
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
void CScrTft::DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
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
void CScrTft::DrawRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int color) {
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
void CScrTft::DrawRoundRect(void) {
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
void CScrTft::DrawCircle(unsigned int xi, unsigned int yi, unsigned char pang, unsigned char pfil, unsigned int pcor) {
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
void CScrTft::InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
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
void CScrTft::SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    DrawRect((x - 1), (y - 1), (pwidth + 2), (pheight + 2), Red);
}

//-----------------------------------------------------------------------------
void CScrTft::PutImage(unsigned char* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    (void)(vimage);
    (void)(x);
    (void)(y);
    (void)(pwidth);
    (void)(pheight);

    /* ver como fazer */
}

//-----------------------------------------------------------------------------
CScrTft *CScrTft::Get (void)
{
    assert (s_pThis != 0);
    return s_pThis;
}
