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
    return TRUE;
}
