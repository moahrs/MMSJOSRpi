#ifndef _circle_lib_h
#define _circle_lib_h

#define INQUIRY_SECONDS     20
#define __USE_TFT_LCD__
//#define __USE_CIRCLE_BLUETOOTH__
//#define __USE_BLUETOOTH_ZERO_W__
//#define __USE_WIFI_ZERO_W__
#define __USE_SERIAL_BLUETOOTH__
#define __USE_WIFI_ESP8266__

#include <circle/usb/usbkeyboard.h>
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
#include <../lib/circlelib/addon/SDCard/emmc.h>
#include <../lib/circlelib/addon/fatfs/ff.h>
#include <kernel/mmsjos.h>

#endif
