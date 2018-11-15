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

/*-UsbInitialise-------------------------------------------------------------
 Initialises the USB driver by performing necessary interfactions with the
 host controller driver, and enumerating the initial device tree.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
RESULT UsbInitialise (void);

/*-UsbGetRootHub ------------------------------------------------------------
 On a Universal Serial Bus, there exists a root hub. This if often a virtual
 device, and typically represents a one port hub, which is the physical
 universal serial bus for this computer. It is always address 1. It is present 
 to allow uniform software manipulation of the universal serial bus itself.
 This will return that FAKE rootHub or NULL on failure. Reason for failure is
 generally not having called USBInitialize to start the USB system.         
 11Apr17 LdB
 --------------------------------------------------------------------------*/
struct UsbDevice *UsbGetRootHub (void);

/*-UsbShowTree --------------------------------------------------------------
 Shows the USB tree as ascii art using the Printf command. The normal command
 to show from roothub up is UsbShowTree(UsbGetRootHub(), 1, '+');
 14Mar17 LdB
 --------------------------------------------------------------------------*/
void UsbShowTree (struct UsbDevice *root, const int level, const char tee);

/*-IsHub---------------------------------------------------------------------
 Will return if the given usbdevice is infact a hub and thus has hub payload
 data available. Remember the gateway node of a hub is a normal usb device.
 You should always call this first up in any routine that accesses the hub
 payload to make sure the payload pointers are valid. If it returns true it
 is safe to proceed and do things with the hub payload via it's pointer.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
bool IsHub (uint8_t devNumber);

/*-IsHid---------------------------------------------------------------------
 Will return if the given usbdevice is infact a hid and thus has hid payload
 data available. Remember a hid device is a normal usb device which takes
 human input (like keyboard, mouse etc). You should always call this first
 in any routine that accesses the hid payload to make sure the pointers are
 valid. If it returns true it is safe to proceed and do things with the hid
 payload via it's pointer.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
bool IsHid (uint8_t devNumber);

/*-IsMassStorage------------------------------------------------------------
 Will return if the given usbdevice is infact a mass storage device and thus
 has a mass storage payload data available. You should always call this first
 in any routine that accesses the storage payload to make sure the pointers
 are valid. If it returns true it is safe to proceed and do things with the
 storage payload via it's pointer.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
bool IsMassStorage (uint8_t devNumber);

/*-IsMouse-------------------------------------------------------------------
 Will return if the given usbdevice is infact a mouse. This initially checks
 the device IsHid and then refines that down to looking at the interface and
 checking it is defined as a mouse.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
bool IsMouse (uint8_t devNumber);

/*-IsKeyboard----------------------------------------------------------------
 Will return if the given usbdevice is infact a keyboard. This initially will
 check the device IsHid and then refines that down to looking at the interface 
 and checking it is defined as a keyboard.
 24Feb17 LdB
 --------------------------------------------------------------------------*/
bool IsKeyboard (uint8_t devNumber);

/*- HIDReadReport ----------------------------------------------------------
 Reads the HID report from the given device. The call will error if device
 is not a HID device, you can always check that by the use of IsHID.
 23Mar17 LdB
 --------------------------------------------------------------------------*/
RESULT HIDReadReport (uint8_t devNumber,							// Device number (address) of the device to read
					  uint8_t hidIndex,								// Which hid configuration information is requested from
					  uint16_t reportValue,							// Hi byte = enum HidReportType  Lo Byte = Report Index (0 = default)  
					  uint8_t* Buffer,								// Pointer to a buffer to recieve the report
					  uint16_t Length);								// Length of the report



#define MaximumDevices 32           // Max number of devices with a USB node we will allow
extern void USB_EN_INT(void);
extern void UsbCheckForChange(void);

#ifdef __cplusplus
}
#endif

#endif						// end RPI_USB guard
