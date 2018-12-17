#include <circle/string.h>
#include <circle/types.h>
#include <circle/interrupt.h>
#include <circle/input/keyboardbuffer.h>
#include <ScreenTFT/screentft.h>
#include <ScreenTFT/gui.h>
#include <circle/devicenameservice.h>
#include <circle/sysconfig.h>
#include <circle/util.h>
#include <circle/alloc.h>
#include <circle/timer.h>
#include <stddef.h>
#include <stdint.h>
#include "common/stdlib.h"

#define __USE_TFT_LCD__

CWindow *CWindow::s_pThis = 0;

CWindow::CWindow (CScrTft *mScrTft, char* ptitle, unsigned int x1, unsigned int y1, unsigned int pwidth, unsigned int pheight, unsigned int pcolorf, unsigned int pcolorb)
: m_pScrTft (mScrTft)
{
    m_pScrTft->SaveScreen(x1, y1, pwidth + 5, pheight + 5);

    vWindow.wPosX = x1;
    vWindow.wPosY = y1;
    vWindow.wWidth = pwidth;
    vWindow.wHeight = pheight;
    vWindow.wColorf = pcolorf;
    vWindow.wColorb = pcolorb;
    vWindow.wTitle = ptitle;
    vWindow.wButtons = 0;

    vCountElements = 0;

    s_pThis = this;
}

CWindow::~CWindow (void)
{
    m_pScrTft->RestoreScreen(vWindow.wPosX, vWindow.wPosY, vWindow.wWidth + 5, vWindow.wHeight + 5);

    s_pThis = 0;
    m_pScrTft = 0;
}

//-------------------------------------------------------------------------
CWindow *CWindow::Get (void)
{
    assert (s_pThis != 0);
    return s_pThis;
}

//-------------------------------------------------------------------------
unsigned char CWindow::addElement(unsigned char pElement, unsigned int pPosX, unsigned int pPosY, unsigned int pWidth, unsigned int pHeight, unsigned int pcolorf, unsigned int pcolorb, unsigned char* pString, int pnumopc, ...)
{
    int ix;
    va_list vopc;

    vElements[vCountElements].weType = pElement;
    vElements[vCountElements].wePosX = vWindow.wPosX + pPosX;
    vElements[vCountElements].wePosY = vWindow.wPosY + pPosY;
    vElements[vCountElements].weWidth = pWidth;
    vElements[vCountElements].weHeight = pHeight;
    vElements[vCountElements].weColorf = pcolorf;
    vElements[vCountElements].weColorb = pcolorb;

    va_start(vopc, pnumopc);
    for(ix = 0; ix < pnumopc; ix++)
        vElements[vCountElements].weOpc[ix] = va_arg(vopc, int);
    va_end(vopc);

    ix = 0;
    while(*pString)
        vElements[vCountElements].weString[ix++] = (char)*pString++;
    vElements[vCountElements].weString[ix] = 0x00;

    if (pElement == GUIRADIOSET || pElement == GUITOGGLEBOX)
        vElements[vCountElements].weReturn[0] = (char)vElements[vCountElements].weOpc[0];
    else
        vElements[vCountElements].weReturn[0] = 0x00;

    vCountElements++;

    return (vCountElements - 1);
}

//-------------------------------------------------------------------------
unsigned char CWindow::run(unsigned int ptime, int pnumbuttons, ...)
{
    unsigned int i, xib, yib, pHandle = 0;
    char *bstr, vwb = 0, ii;
    va_list vbutons;

    // Desenha a Janela
    m_pScrTft->FillRect(vWindow.wPosX, vWindow.wPosY, vWindow.wWidth, vWindow.wHeight, vWindow.wColorb);
    m_pScrTft->DrawRect(vWindow.wPosX, vWindow.wPosY, vWindow.wWidth, vWindow.wHeight, vWindow.wColorf);

    if (*vWindow.wTitle) 
    {
        m_pScrTft->DrawRect(vWindow.wPosX, vWindow.wPosY, vWindow.wWidth, 12, vWindow.wColorf);
        m_pScrTft->writesxy(vWindow.wPosX + 2, vWindow.wPosY + 3,8,(char*)vWindow.wTitle,vWindow.wColorf,vWindow.wColorb);
    }

    // Desenha Elementos
    for (ii = 0; ii < vCountElements; ii++)
    {
        switch(vElements[ii].weType)
        {
            case GUIFILLIN:
                fillin(ii, WINDISP);
                break;
            case GUIRADIOSET:
                radioset(ii, WINDISP);
                break;
            case GUITOGGLEBOX:
                togglebox(ii, WINDISP);
                break;
            case GUICOMBOBOX:
                combobox(ii, WINDISP);
                break;
            case GUIEDITOR:
                editor(ii, WINDISP);
                break;
            case GUITEXT:
                m_pScrTft->writesxy(vElements[ii].wePosX, vElements[ii].wePosY,8,(char*)vElements[ii].weString,vElements[ii].weColorf,vElements[ii].weColorb);
                break;
            case GUIBUTTON:
                button(ii, WINDISP);
                break;
            case GUISPIN:
                spin(ii, WINDISP);
                break;
        }
    }

    // Aguarda Tempo ou Botoes
    if (!ptime)
    {
        while (!vwb)
        {
            m_pScrTft->VerifyTouchLcd(NOWHAITTOUCH, &vpostx, &vposty);

            for (ii = 0; ii < vCountElements; ii++)
            {
                switch(vElements[ii].weType)
                {
                    case GUIFILLIN:
                        fillin(ii, WINOPER);
                        break;
                    case GUIRADIOSET:
                        radioset(ii, WINOPER);
                        break;
                    case GUITOGGLEBOX:
                        togglebox(ii, WINOPER);
                        break;
                    case GUICOMBOBOX:
                        combobox(ii, WINOPER);
                        break;
                    case GUIEDITOR:
                        editor(ii, WINOPER);
                        break;
                    case GUIBUTTON:
                        button(ii, WINOPER);

                        if (vElements[ii].weReturn[0] == 1)
                        {
                            va_start(vbutons, pnumbuttons);
                            for(i = 0; i < pnumbuttons; i++)
                            {
                                if (ii == va_arg(vbutons, int))
                                    vwb = ii;
                            }
                            va_end(vbutons);
                        }

                        break;
                    case GUISPIN:
                        spin(ii, WINOPER);
                        break;
                }
            }
        }
    }
    else
        CTimer::Get ()->MsDelay(ptime * 1000);        

    return vwb;
}

//-------------------------------------------------------------------------
unsigned char* CWindow::GetReturnElement(unsigned char pElement)
{
    if (vElements[pElement].weType == GUIFILLIN)
        return (unsigned char*)vElements[pElement].weString;
    else
        return (unsigned char*)vElements[pElement].weReturn;
} 

//-------------------------------------------------------------------------
void CWindow::radioset(unsigned char pElement, unsigned char vtipo) {
  unsigned char cc, xc, ix;
  unsigned char cchar, vdisp = 0;

  xc = 0;
  cc = 0;
  cchar = ' ';

  while(vtipo == WINOPER && cchar != '\0') {
    cchar = vElements[pElement].weString[cc];
    if (cchar == ',') {
      if (cchar == ',' && cc != 0)
        xc++;

      if (vpostx >= vElements[pElement].wePosX && vpostx <= (vElements[pElement].wePosX + vElements[pElement].weWidth) && vposty >= (vElements[pElement].wePosY + (xc * 10)) && vposty <= ((vElements[pElement].wePosY + (xc * 10)) + 8)) {
        vElements[pElement].weReturn[0] = xc;
        vdisp = 1;
      }
    }

    cc++;
  }

  xc = 0;
  cc = 0;
  ix = 0;

  while(vtipo == WINDISP || vdisp) 
  {
    cchar = vElements[pElement].weString[cc];

    if (cchar == ',') 
    {
      if (vElements[pElement].weWidth < ix)
        vElements[pElement].weWidth = ix;

      ix = 0;

      if (cchar == ',' && cc != 0)
        xc++;

      m_pScrTft->FillRect(vElements[pElement].wePosX, vElements[pElement].wePosY + (xc * 10), 8, 8, vElements[pElement].weColorb);
      m_pScrTft->DrawCircle(vElements[pElement].wePosX + 4, vElements[pElement].wePosY + (xc * 10) + 2, 4, 0, vElements[pElement].weColorf);

      if (vElements[pElement].weReturn[0] == xc)
        m_pScrTft->DrawCircle(vElements[pElement].wePosX + 4, vElements[pElement].wePosY + (xc * 10) + 2, 3, 1, vElements[pElement].weColorf);
      else
        m_pScrTft->DrawCircle(vElements[pElement].wePosX + 4, vElements[pElement].wePosY + (xc * 10) + 2, 3, 0, vElements[pElement].weColorf);

      m_pScrTft->locatexy(vElements[pElement].wePosX + 10, vElements[pElement].wePosY + (xc * 10));
    }

    if (cchar != ',' && cchar != '\0')
    {
      m_pScrTft->writecxy(8, cchar, vElements[pElement].weColorf, vElements[pElement].weColorb);
      ix += 10;
    }

    if (cchar == '\0')
    {
      if (vElements[pElement].weWidth < ix)
        vElements[pElement].weWidth = ix;

      break;
    }

    cc++;
  }
}

//-------------------------------------------------------------------------
void CWindow::togglebox(unsigned char pElement, unsigned char vtipo) {
  unsigned char cc = 0;
  unsigned char cchar, vdisp = 0;

  if (vtipo == WINOPER && vpostx >=  vElements[pElement].wePosX && vpostx <= vElements[pElement].wePosX + 4 && vposty >= vElements[pElement].wePosY && vposty <= vElements[pElement].wePosY + 4) {
    if (vElements[pElement].weReturn[0])
      vElements[pElement].weReturn[0] = 0;
    else
      vElements[pElement].weReturn[0] = 1;

    vdisp = 1;
  }

  if (vtipo == WINDISP || vdisp) {
    m_pScrTft->FillRect(vElements[pElement].wePosX, vElements[pElement].wePosY + 2, 4, 4, vElements[pElement].weColorb);
    m_pScrTft->DrawRect(vElements[pElement].wePosX, vElements[pElement].wePosY + 2, 4, 4, vElements[pElement].weColorf);

    if (vElements[pElement].weReturn[0]) {
      m_pScrTft->DrawLine(vElements[pElement].wePosX, vElements[pElement].wePosY + 2, vElements[pElement].wePosX + 4, vElements[pElement].wePosY + 6, vElements[pElement].weColorf);
      m_pScrTft->DrawLine(vElements[pElement].wePosX, vElements[pElement].wePosY + 6, vElements[pElement].wePosX + 4, vElements[pElement].wePosY + 2, vElements[pElement].weColorf);
    }  

    if (vtipo == WINDISP) {
      vElements[pElement].wePosX += 6;
      m_pScrTft->locatexy(vElements[pElement].wePosX,vElements[pElement].wePosY);
      while (vElements[pElement].weString[cc] != 0) {
        cchar = vElements[pElement].weString[cc];
        cc++;

        m_pScrTft->writecxy(8, cchar, vElements[pElement].weColorf, vElements[pElement].weColorb);
        vElements[pElement].wePosX += 6;
      }
    }
  }
}

//-------------------------------------------------------------------------
void CWindow::fillin(unsigned char pElement, unsigned char vtipo)
{
    unsigned int cc = 0, ikx, nResult = 0, vkeybuffer[100];
    unsigned char cchar, *vvarptr, vdisp = 0;

    vvarptr = (unsigned char*)vElements[pElement].weString;

    while (*vvarptr) {
        cc += 8;
        vvarptr++;
    }

    if (vtipo == WINOPER) 
    {
        if (vElements[pElement].weOpc[0])
            vdisp = 2;
        else
        {
            // ver como fazer pra usar teclado/touch aqui....
        }
    }

    if (vtipo == WINDISP || vdisp) {
        vElements[pElement].weOpc[0] = 0;

        if (!vdisp || vdisp == 2) {
            m_pScrTft->DrawRect(vElements[pElement].wePosX,vElements[pElement].wePosY,vElements[pElement].weWidth,vElements[pElement].weHeight,vElements[pElement].weColorf);
            m_pScrTft->FillRect(vElements[pElement].wePosX+1,vElements[pElement].wePosY+1,vElements[pElement].weWidth-2,8,vElements[pElement].weColorb);
        }

        vvarptr = (unsigned char*)vElements[pElement].weString;
        m_pScrTft->locatexy(vElements[pElement].wePosX+2,vElements[pElement].wePosY+2);
        while (*vvarptr) {
            cchar = *vvarptr++;
            cc++;

            m_pScrTft->writecxy(8, cchar, vElements[pElement].weColorf, vElements[pElement].weColorb);

            if (m_pScrTft->GetPosX() >= vElements[pElement].wePosX + vElements[pElement].weWidth)
                break;
        }
    }
}

//-------------------------------------------------------------------------
void CWindow::spin(unsigned char pElement, unsigned char vtipo)
{
    int iy, pelemc, vval;

    if (vtipo == WINOPER) 
    {
        pelemc = vElements[pElement].weOpc[0];
        if (vpostx >= vElements[pElement].wePosX && vpostx <= (vElements[pElement].wePosX + 12))
        {
            if (vposty >= (vElements[pElement].wePosY - 1) && vposty <= (vElements[pElement].wePosY + 6))
            {
                vval = atoi(vElements[pelemc].weString);

                if (vval + 1 <= vElements[pElement].weOpc[2])
                {
                    vval++;
                    itoa(vval, vElements[pelemc].weString, 10);
                    vElements[pelemc].weOpc[0] = 1;
                }
            } 
            else if (vposty >= (vElements[pElement].wePosY + 9) && vposty <= (vElements[pElement].wePosY + 14))
            {
                vval = atoi(vElements[pelemc].weString);

                if ((vval - 1) >= vElements[pElement].weOpc[1])
                {
                    vval--;
                    itoa(vval, vElements[pelemc].weString, 10);
                    vElements[pelemc].weOpc[0] = 1;
                }
            }
        }
    }

    if (vtipo == WINDISP) 
    {
        m_pScrTft->FillRect(vElements[pElement].wePosX, vElements[pElement].wePosY, 12, 15, vElements[pElement].weColorb);
        m_pScrTft->DrawRect(vElements[pElement].wePosX, vElements[pElement].wePosY, 12, 15, vElements[pElement].weColorf); 
        m_pScrTft->DrawLine(vElements[pElement].wePosX, vElements[pElement].wePosY + 8 ,vElements[pElement].wePosX + 12, vElements[pElement].wePosY + 8 , vElements[pElement].weColorf);

        // Desenha "/\" e "\/"
        for(iy = 0; iy <= 4; iy++)
        {
            m_pScrTft->DrawLine(vElements[pElement].wePosX + 6 - iy, vElements[pElement].wePosY + 2 + iy,vElements[pElement].wePosX + 6 + iy, vElements[pElement].wePosY + 2 + iy, vElements[pElement].weColorf);
            m_pScrTft->DrawLine(vElements[pElement].wePosX + 6 - iy, vElements[pElement].wePosY + 14 - iy,vElements[pElement].wePosX + 6 + iy, vElements[pElement].wePosY + 14 - iy, vElements[pElement].weColorf);
        }
    }
}

//-------------------------------------------------------------------------
void CWindow::button(unsigned char pElement, unsigned char vtipo)
{
    if (vtipo == WINOPER && vpostx >= vElements[pElement].wePosX && vpostx <= (vElements[pElement].wePosX + vElements[pElement].weWidth) && vposty >= vElements[pElement].wePosY && vposty <= (vElements[pElement].wePosY + vElements[pElement].weHeight)) 
    {
        vElements[pElement].weReturn[0] = 1;
    }

    if (vtipo == WINDISP) {
        m_pScrTft->FillRect(vElements[pElement].wePosX, vElements[pElement].wePosY, vElements[pElement].weWidth, vElements[pElement].weHeight, vElements[pElement].weColorb);
        m_pScrTft->DrawRoundRect(vElements[pElement].wePosX, vElements[pElement].wePosY, vElements[pElement].weWidth, vElements[pElement].weHeight, 1, vElements[pElement].weColorf); 
        m_pScrTft->writesxy(vElements[pElement].wePosX + 2, vElements[pElement].wePosY + 2,8,vElements[pElement].weString,vElements[pElement].weColorf,vElements[pElement].weColorb);
    }
}

//-------------------------------------------------------------------------
void CWindow::slider(unsigned char pElement, unsigned char vtipo)
{

}

//-------------------------------------------------------------------------
void CWindow::combobox(unsigned char pElement, unsigned char vtipo)
{

}

//-------------------------------------------------------------------------
void CWindow::editor(unsigned char pElement, unsigned char vtipo)
{

}
