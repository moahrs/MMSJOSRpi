#ifndef _circle_lib_h
#define _circle_lib_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
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
#include <drivers/lcd_vdg.h>
#include <drivers/lcd_tch.h>
#include <drivers/screentft.h>
#include <kernel/mmsjos.h>

#define INQUIRY_SECONDS     20
#define __USE_TFT_LCD__

#endif