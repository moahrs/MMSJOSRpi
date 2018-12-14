#include <circle/string.h>
#include <circle/types.h>
#include <circle/interrupt.h>
#include <ScreenTFT/screentft.h>
#include <circle/devicenameservice.h>
#include <circle/sysconfig.h>
#include <circle/util.h>
#include <circle/alloc.h>
#include <stddef.h>
#include <stdint.h>
#include "../../../include/common/mylib.h"

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
    vpagescr = 1;

    CDeviceNameService::Get ()->AddDevice ("ScrLdg", this, FALSE);

    return true;
}

//-----------------------------------------------------------------------------
unsigned CScrTft::GetOutput (void) const
{
    return voutput;
}

//-----------------------------------------------------------------------------
void CScrTft::SetTypeKeyboard(unsigned pTypeKey)
{
    pKeyboardType = pTypeKey;
}

//-----------------------------------------------------------------------------
void CScrTft::SetOutput (unsigned pOutput)
{
    voutput = pOutput;
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
    p_mLcdVdg->TFT_Fill(pcolor);

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
        p_mLcdVdg->TFT_WriteChar(pbyte, ((vlin * 10) + vovery), ((vcol * 8) + voverx), 8, xcolor, xbcolor);

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
            p_mLcdVdg->TFT_Scroll(10, vcorb);
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
void CScrTft::getPos(unsigned int *pPostX, unsigned int *pPostY)
{
    paramVDG[0] =  0x01;
    paramVDG[1] =  0xDD;
    p_mLcdVdg->commVDG(paramVDG);    

    *pPostY = (paramVDG[0] << 8) | paramVDG[1];
    *pPostX = (paramVDG[2] << 8) | paramVDG[3];
}

//-----------------------------------------------------------------------------
void CScrTft::writesxy(unsigned int x, unsigned int y, unsigned char sizef, char *msgs, unsigned int pcolor, unsigned int pbcolor) {
    unsigned char ix = 0, ss[255];
    unsigned int iy;

    while (*msgs)
    {
        if (*msgs >= 20 && *msgs <= 0x7F)
            ss[ix++] = *msgs;
        msgs++;
    }

    ss[ix] = 0x00;

    p_mLcdVdg->TFT_Text((char*)ss,y,x,sizef,pcolor,pbcolor);
}

//-----------------------------------------------------------------------------
void CScrTft::writecxy(unsigned char sizef, unsigned char pbyte, unsigned int pcolor, unsigned int pbcolor) 
{
    p_mLcdVdg->TFT_WriteChar(pbyte, pposy, pposx, sizef, pcolor, pbcolor);

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
void CScrTft::SaveScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight) {
    if (vpagescr <= 8)
        p_mLcdVdg->TFT_SaveScreen(vpagescr++, yi, xi, pwidth, pheight);
}

//-----------------------------------------------------------------------------
void CScrTft::RestoreScreen(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight) {
    p_mLcdVdg->TFT_RestoreScreen(--vpagescr, yi, xi, pwidth, pheight);

    if (vpagescr < 1)
        vpagescr = 1;
}

//-----------------------------------------------------------------------------
void CScrTft::SetDot(unsigned int x, unsigned int y, unsigned int color) {
    p_mLcdVdg->TFT_Dot(y, x, color);
}

//-----------------------------------------------------------------------------
void CScrTft::FillRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int pcor) {
    unsigned int xf, yf;

    xf = (xi + pwidth);
    yf = (yi + pheight);

    p_mLcdVdg->TFT_Box(yi, xi, yf, xf, pcor);
}

//-----------------------------------------------------------------------------
void CScrTft::DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int color) {
    p_mLcdVdg->TFT_Line(y1, x1, y2, x2, color);
}

//-----------------------------------------------------------------------------
void CScrTft::DrawRect(unsigned int xi, unsigned int yi, unsigned int pwidth, unsigned int pheight, unsigned int color) {
    unsigned int xf, yf;

    xf = (xi + pwidth);
    yf = (yi + pheight);

    p_mLcdVdg->TFT_Rectangle(yi, xi, yf, xf, color);
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
    p_mLcdVdg->TFT_Circle(yi, xi, pang, pfil, pcor);
}

//-----------------------------------------------------------------------------
void CScrTft::InvertRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    p_mLcdVdg->TFT_InvertRect(y, x, pwidth, pheight);
}

//-----------------------------------------------------------------------------
void CScrTft::SelRect(unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    DrawRect((x - 1), (y - 1), (pwidth + 2), (pheight + 2), Red);
}

//-----------------------------------------------------------------------------
void CScrTft::PutImage(unsigned int* vimage, unsigned int x, unsigned int y, unsigned int pwidth, unsigned int pheight) {
    p_mLcdVdg->TFT_Show_Image(y, x, pwidth, pheight, vimage);
}

//-----------------------------------------------------------------------------
CScrTft *CScrTft::Get (void)
{
    assert (s_pThis != 0);
    return s_pThis;
}

//------------------------------------------------------------------------
void CScrTft::VerifyTouchLcd(unsigned char vtipo, unsigned int *ppostx, unsigned int *pposty) 
{
    unsigned int vbytepic = 0;

    vbytepic = 0;

    *ppostx = 0;
    *pposty = 0;

    while (!vbytepic) 
    {
        vbytepic = getKey();

        if (vbytepic == 0xFF) 
            getPos(ppostx, pposty);

        if (!vtipo)
            break;
    }
}

//------------------------------------------------------------------------
void CScrTft::showWindow(unsigned char* vparamstrscr, unsigned int *vparamscr)
{
    unsigned int i, ii, xib, yib, x1, y1, pwidth, pheight;
    unsigned char *bstr, bbutton;

    bstr = vparamstrscr;
    x1 = vparamscr[0];
    y1 = vparamscr[1];
    pwidth = vparamscr[2];
    pheight = vparamscr[3];
    bbutton = vparamscr[4];

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
void CScrTft::drawButtons(unsigned int xib, unsigned int yib) {

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
unsigned char CScrTft::waitButton(void) {
  unsigned char i, ii, iii;

  ii = 0;
  VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

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
unsigned char CScrTft::message(char* bstr, unsigned char bbutton, unsigned int btime)
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
    SaveScreen(xi,yi,pwidth,pheight);

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
          VerifyTouchLcd(WHAITTOUCH, &vpostx, &vposty);

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

      RestoreScreen(xi,yi,pwidth,pheight);

    return ii;
}


//-------------------------------------------------------------------------
void CScrTft::radioset(unsigned char* vopt, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
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
void CScrTft::togglebox(unsigned char* bstr, unsigned char *vvar, unsigned int x, unsigned int y, unsigned char vtipo) {
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
void CScrTft::fillin(unsigned char* vvar, unsigned int x, unsigned int y, unsigned int pwidth, unsigned char vtipo)
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
}

void CScrTft::showImageBMP(unsigned int posx, unsigned int posy, unsigned int pwidth, unsigned int pheight, unsigned char* pfileImage)
{
    BITMAPINFOHEADER *bitmapInfoHeader;
    BITMAPFILEHEADER *bitmapFileHeader; //our bitmap file header
    unsigned char *bitmapImage;  //store image data
    int ix, iy, imageIdx=0, imageIx, imageStart;  //image index counter
    unsigned char tempRGB;  //our swap variable
    unsigned int *vimageshow = (unsigned int*)malloc(180000);
    unsigned char vptrfileinfo[sizeof(BITMAPFILEHEADER)];
    unsigned char vptrinfo[sizeof(BITMAPINFOHEADER)];
    unsigned char *vfileImage = (unsigned char*)(pfileImage + (sizeof(BITMAPFILEHEADER) + 1));
    unsigned int plinha, pw = pwidth, ph = pheight;

    // Ler File Info Header
    memcpy((unsigned char*)&vptrfileinfo, (unsigned char*)pfileImage, sizeof(BITMAPFILEHEADER));
    bitmapFileHeader = (BITMAPFILEHEADER*)vptrfileinfo;

    // Ler Head Info Header    
    memcpy((unsigned char*)&vptrinfo, (unsigned char*)(pfileImage + sizeof(BITMAPFILEHEADER)), sizeof(BITMAPINFOHEADER));
    bitmapInfoHeader = (BITMAPINFOHEADER*)vptrinfo;

    imageStart = bitmapFileHeader->bfOffBits;

    if (bitmapFileHeader->bfType == 0x4D42)
    {
        //swap the r and b values to get RGB (bitmap is BGR)
        plinha = pw * 3;
        imageIx = 0;
        for (iy = 1; iy <= ph; iy++) // fixed semicolon
        {
            imageIdx = bitmapInfoHeader->biSizeImage - (plinha * iy);
            for (ix = imageIdx; ix < (imageIdx + plinha); ix += 3)
            {
                vimageshow[imageIx++] = ((pfileImage[imageStart + ix + 2] >> 3) << 11)  | ((pfileImage[imageStart + ix + 1] >> 2) << 5) | (pfileImage[imageStart + ix] >> 3);
            }
        }

        p_mLcdVdg->TFT_Show_Image(posy,posx,pw,ph,vimageshow);
    }
}

//-------------------------------------------------------------------------
/*void combobox(unsigned char* vopt, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/

//-------------------------------------------------------------------------
/*void editor(unsigned char* vtexto, unsigned char *vvar,unsigned char x, unsigned char y, unsigned char vtipo) {
}*/
