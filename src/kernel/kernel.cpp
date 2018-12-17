//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <circle/input/keyboardbuffer.h>
#include <kernel/kernel.h>
#include <circle/usb/usb.h>
#include <circle/debug.h>
#include <circle/nulldevice.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <circle/time.h>
#include <drivers/ds1307.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
	#ifdef __USE_TFT_LCD__
:		m_ScrTft (&m_Interrupt),
	#else
:		m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	#endif
	m_Timer (&m_Interrupt),
	m_Logger (9999, &m_Timer),
	m_DWHCI (&m_Interrupt, &m_Timer),
	m_Bluetooth (&m_Interrupt),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_MMSJOS (&m_Interrupt, &m_Timer, &m_DWHCI, &m_FileSystem, &m_EMMC)
{
	//	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
    time_t pData;
    unsigned nTime;

	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	#ifdef __USE_TFT_LCD__
	    if (bOK)
	    {
	        bOK = m_ScrTft.Initialize ();
	    }
	#else
		if (bOK)
		{
			bOK = m_Screen.Initialize ();
		}
	#endif

	if (bOK)
	{
		#ifdef __USE_TFT_LCD__
			CDevice *pTarget = &m_ScrTft;
		#else
			CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
			if (pTarget == 0)
			{
				pTarget = &m_Screen;
			}
		#endif

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		m_Logger.Write (FromKernel, LogNotice, "Initializing Timer...");
		bOK = m_Timer.Initialize ();

	    ds1307_init(); 
        pData = ds1307_read();
        if (!pData.Error) 
        {
        	nTime = 0;

			m_Logger.Write (FromKernel, LogNotice, "RTC %d/%d/%d %d:%d:%d",bcd2bin(pData.Month),bcd2bin(pData.Day),bcd2bin(pData.Year)+2000,bcd2bin(pData.Hour),bcd2bin(pData.Minute),bcd2bin(pData.Second));

			for (unsigned nYear = 1970; nYear < (unsigned)bcd2bin(pData.Year)+2000; nYear++)
			{
				nTime += CTime::IsLeapYear (nYear) ? 366 : 365;
			}

			for (unsigned nMonth = 0; nMonth < (unsigned)(bcd2bin(pData.Month) - 1); nMonth++)
			{
				nTime += CTime::GetDaysOfMonth (nMonth, bcd2bin(pData.Year)+2000);
			}

			nTime += bcd2bin(pData.Day)-1;
			nTime *= 24;
			nTime += bcd2bin(pData.Hour);
			nTime *= 60;
			nTime += bcd2bin(pData.Minute);
			nTime *= 60;
			nTime += bcd2bin(pData.Second);

			m_Timer.SetTime (nTime, FALSE);
        }
	}

	if (bOK)
	{
		m_Logger.Write (FromKernel, LogNotice, "Initializing USB...");
		bOK = m_DWHCI.Initialize ();
	}

	if (bOK)
	{
		m_Logger.Write (FromKernel, LogNotice, "Initializing EMMC...");
		bOK = m_EMMC.Initialize ();
	}

    if (bOK)
    {
        bOK = m_MMSJOS.Initialize ();
    }

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	m_MMSJOS.Start();

	return ShutdownHalt;
}
