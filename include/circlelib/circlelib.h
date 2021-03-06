#ifndef _circle_lib_h
#define _circle_lib_h

#define INQUIRY_SECONDS     60
#define INQUIRY_LIMITED_DISCOVERY   1
#define INQUIRY_FIND_DEVICES        0

#define __USE_TFT_LCD__

#include <circle/usb/usbkeyboard.h>
#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/dwhcidevice.h>
#include <circle/types.h>
#include <circle/sched/scheduler.h>
#include <circle/bt/btsubsystem.h>
#include <circle/bcmpropertytags.h>
#include <circle/fs/fat/fatfs.h>
#include <circle/device.h>

#ifdef __USE_TFT_LCD__
	#include <ScreenTFT/screentft.h>
	#include <ScreenTFT/gui.h>
#else
	#include <circle/screen.h>
#endif

#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <kernel/mmsjos.h>

#endif
