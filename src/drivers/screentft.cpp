#include <circle/string.h>
#include <circle/types.h>
#include <drivers/lcd_vdg.h>
#include <drivers/screentft.h>
#include <circle/devicenameservice.h>
#include <circle/sysconfig.h>
#include <circle/util.h>
#include <stddef.h>
#include <stdint.h>
#include <common/mylib.h>

#define __USE_TFT_LCD__

CScrTft *CScrTft::s_pThis = 0;

CScrTft::CScrTft (CLcdVdg *mLcdVdg)
: p_mLcdVdg (mLcdVdg)
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
CScrTft *CScrTft::Get (void)
{
    assert (s_pThis != 0);
    return s_pThis;
}
