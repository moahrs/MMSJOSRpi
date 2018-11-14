#ifndef _RPI_USB_API_					// Check RPI_USB guard
#define _RPI_USB_API_

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	OK = 0,
	ErrorGeneral = -1,
	ErrorArgument = -2,
	ErrorRetry = -3,
	ErrorDevice = -4,
	ErrorIncompatible = -5,
	ErrorCompiler = -6,
	ErrorMemory = -7,
	ErrorTimeout = -8,
	ErrorHardware = -9,
	ErrorTransmission = -10,
	ErrorDisconnected = -11,
	ErrorDeviceNumber = -12,
	ErrorTooManyRetries = -13,
	ErrorIndex = -14,
	ErrorNotHID = -15,
	ErrorStall = -16,
} RESULT;

/*--------------------------------------------------------------------------}
{ 					 USB HID 1.11 defined report types						}
{--------------------------------------------------------------------------*/
enum HidReportType {
	USB_HID_REPORT_TYPE_INPUT = 1,									// Input HID report
	USB_HID_REPORT_TYPE_OUTPUT = 2,									// Output HID report
	USB_HID_REPORT_TYPE_FEATURE = 3,								// Feature HID report
};


#define MaximumDevices 32           // Max number of devices with a USB node we will allow
extern void USB_EN_INIT() ;

#ifdef __cplusplus
}
#endif

#endif						// end RPI_USB guard
