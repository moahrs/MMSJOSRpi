//
// bluetooth.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015-2016  R. Stange <rsta2@o2online.de>
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
#ifndef _circle_bt_bluetooth_h
#define _circle_bt_bluetooth_h

#include <circle/macros.h>
#include <circle/types.h>

////////////////////////////////////////////////////////////////////////////////
//
// General definitions
//
////////////////////////////////////////////////////////////////////////////////

// Class of Device

#define BT_CLASS_DESKTOP_COMPUTER	0x002104

// Sizes

#define BT_MAX_HCI_EVENT_SIZE	257
#define BT_MAX_HCI_COMMAND_SIZE	258
#define BT_MAX_DATA_SIZE	BT_MAX_HCI_COMMAND_SIZE

#define BT_BD_ADDR_SIZE		6
#define BT_CLASS_SIZE		3
#define BT_NAME_SIZE		248

// Error codes

#define BT_STATUS_SUCCESS	0

////////////////////////////////////////////////////////////////////////////////
//
// HCI
//
////////////////////////////////////////////////////////////////////////////////

// Commands
struct TBTHCICommandHeader
{
	u16	OpCode;
#define OGF_LINK_CONTROL		(1 << 10)
	#define OP_CODE_INQUIRY			(OGF_LINK_CONTROL | 0x001)
	#define OP_HCI_ACCEPT_CONNECT_REQUEST			(OGF_LINK_CONTROL | 0x009)
	#define OP_CODE_REMOTE_NAME_REQUEST	(OGF_LINK_CONTROL | 0x019)
	#define OP_CODE_PIN_CODE_REQUEST_REPLY (OGF_LINK_CONTROL | 0x00D)
	#define OP_HCI_ACCEPT_SYNC_CONNECT_REQUEST (OGF_LINK_CONTROL | 0x029)
#define OGF_LINK_POLICY		(2 << 10)
	#define OP_HCI_WRITE_LINK_POLICY_SETTINGS (OGF_LINK_POLICY | 0x00D)
#define OGF_HCI_CONTROL_BASEBAND	(3 << 10)
	#define OP_CODE_RESET			(OGF_HCI_CONTROL_BASEBAND | 0x003)
	#define OP_CODE_WRITE_LOCAL_NAME	(OGF_HCI_CONTROL_BASEBAND | 0x013)
	#define OP_CODE_WRITE_SCAN_ENABLE	(OGF_HCI_CONTROL_BASEBAND | 0x01A)
	#define OP_CODE_WRITE_CLASS_OF_DEVICE	(OGF_HCI_CONTROL_BASEBAND | 0x024)
#define OGF_INFORMATIONAL_COMMANDS	(4 << 10)
	#define OP_CODE_READ_BD_ADDR		(OGF_INFORMATIONAL_COMMANDS | 0x009)
#define OGF_VENDOR_COMMANDS		(0x3F << 10)
	u8	ParameterTotalLength;
}
PACKED;

#define PARM_TOTAL_LEN(cmd)		(sizeof (cmd) - sizeof (TBTHCICommandHeader))

struct TBTHCIInquiryCommand
{
	TBTHCICommandHeader	Header;

	u8	LAP[BT_CLASS_SIZE];
#define INQUIRY_LAP_GIAC		0x9E8B33	// General Inquiry Access Code
#define INQUIRY_LAP_LIAC		0x9E8B00	// Limited Inquiry Access Code
	u8	InquiryLength;
#define INQUIRY_LENGTH_MIN		0x01		// 1.28s
#define INQUIRY_LENGTH_MAX		0x30		// 61.44s
#define INQUIRY_LENGTH(secs)		(((secs) * 100 + 64) / 128)
	u8	NumResponses;
#define INQUIRY_NUM_RESPONSES_UNLIMITED	0x00
}
PACKED;

struct TBTHCIRemoteNameRequestCommand
{
	TBTHCICommandHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8	PageScanRepetitionMode;
#define PAGE_SCAN_REPETITION_R0		0x00
#define PAGE_SCAN_REPETITION_R1		0x01
#define PAGE_SCAN_REPETITION_R2		0x02
	u8	Reserved;				// set to 0
	u16	ClockOffset;
#define CLOCK_OFFSET_INVALID		0		// bit 15 is not set
}
PACKED;

struct TBTHCIAcceptConnectionRequestCommand
{
	TBTHCICommandHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8	RemainSlave;
}
PACKED;

struct TBTHCIWriteLinkPolicySettingsCommand
{
	TBTHCICommandHeader	Header;

	u16	ConnHandle;
	u16 Link_Policy_Settings;
}
PACKED;

struct TBTHCIAcceptSyncConnectionRequestCommand
{
	TBTHCICommandHeader	Header;

	u8  BDAddr[BT_BD_ADDR_SIZE];
	u32 Transmit_Bandwidth;
	u32 Receive_Bandwidth;
	u16 Max_Latency;
	u16 Voice_Setting;
	u8  Retransmission_Effort;
	u16 Packet_Type;
} PACKED;

struct TBTHCIPinCodeRequestReplyCommand
{
	TBTHCICommandHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8	PinCodeLength;
	u8  PinCode[16];
}
PACKED;

struct TBTHCIWriteLocalNameCommand
{
	TBTHCICommandHeader	Header;

	u8	LocalName[BT_NAME_SIZE];
}
PACKED;

struct TBTHCIWriteScanEnableCommand
{
	TBTHCICommandHeader	Header;

	u8	ScanEnable;
#define SCAN_ENABLE_NONE		0x00
#define SCAN_ENABLE_INQUIRY_ENABLED	0x01
#define SCAN_ENABLE_PAGE_ENABLED	0x02
#define SCAN_ENABLE_BOTH_ENABLED	0x03
}
PACKED;

struct TBTHCIWriteClassOfDeviceCommand
{
	TBTHCICommandHeader	Header;

	u8	ClassOfDevice[BT_CLASS_SIZE];
}
PACKED;

// Events

struct TBTHCIEventHeader
{
	u8	EventCode;
#define EVENT_CODE_INQUIRY_COMPLETE		0x01
#define EVENT_CODE_INQUIRY_RESULT		0x02
#define EVENT_CODE_CONNECT_COMPLETE     0x03
#define EVENT_CODE_CONNECT_REQUEST		0x04
#define EVENT_CODE_DISCONNECT   		0x05
#define EVENT_CODE_AUTH_COMPLETE		0x06
#define EVENT_CODE_REMOTE_NAME_REQUEST_COMPLETE	0x07
#define EVENT_ENCRYPTION_CHANGE         0x08
#define EVENT_CODE_COMMAND_COMPLETE		0x0E
#define EVENT_CODE_COMMAND_STATUS		0x0F
#define EVENT_CODE_PIN_CODE_REQUEST     0x16
#define EVENT_LINK_KEY_NOTIFICATION		0x18
#define EVENT_CODE_MAX_SLOTS_CHANGE     0x1B
#define EVENT_CODE_SYNC_CONNECT_COMPLETE 0x2C
	u8	ParameterTotalLength;
}
PACKED;

struct TBTHCIEventInquiryComplete
{
	TBTHCIEventHeader	Header;

	u8	Status;
}
PACKED;

struct TBTHCIEventInquiryResult
{
	TBTHCIEventHeader	Header;

	u8	NumResponses;

//	u8	BDAddr[NumResponses][BT_BD_ADDR_SIZE];
//	u8	PageScanRepetitionMode[NumResponses];
//	u8	Reserved[NumResponses][2];
//	u8	ClassOfDevice[NumResponses][BT_CLASS_SIZE];
//	u16	ClockOffset[NumResponses];
	u8	Data[0];
#define INQUIRY_RESP_SIZE			14
#define INQUIRY_RESP_BD_ADDR(p, i)		(&(p)->Data[(i)*BT_BD_ADDR_SIZE])
#define INQUIRY_RESP_PAGE_SCAN_REP_MODE(p, i)	((p)->Data[(p)->NumResponses*BT_BD_ADDR_SIZE + (i)])
#define INQUIRY_RESP_CLASS_OF_DEVICE(p, i)	(&(p)->Data[(p)->NumResponses*(BT_BD_ADDR_SIZE+1+2) \
							   + (i)*BT_CLASS_SIZE])
}
PACKED;

struct TBTHCIEventPinCodeRequest
{
	TBTHCIEventHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
}
PACKED;

struct TBTHCIEventConnectRequest
{
	TBTHCIEventHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8  ClassOfDevice;
	u8  LinkType;
}
PACKED;

struct TBTHCIEventMaxSlotsChange
{
	TBTHCIEventHeader	Header;

	u16 ConnHandle;
	u8  LMPMaxSlots;
}
PACKED;

struct TBTHCIEventAuthComplete
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u16 ConnHandle;
}
PACKED;

struct TBTHCIEventEncryptionChange
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u16 ConnHandle;
	u8  Encryption_Enabled;
}
PACKED;

struct TBTHCIEventLinkKeyNotification
{
	TBTHCIEventHeader	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8  Link_Key[16];
	u8  Key_Type;
}
PACKED;

struct TBTHCIEventDisconnect
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u16 ConnHandle;
	u8  Reason;
}
PACKED;

struct TBTHCIEventConnectComplete
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u16 ConnHandle;
	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8  LinkType;
	u8  EncryptEnabled;
}
PACKED;

struct TBTHCIEventSyncConnectComplete
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u16 ConnHandle;
	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8  LinkType;
	u8  Transmission_Interval;
	u8  Retransmission_Window;
	u16 Rx_Packet_Length;
	u16 Tx_Packet_Length;
	u8  Air_Mode;
}
PACKED;

struct TBTHCIEventRemoteNameRequestComplete
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u8	BDAddr[BT_BD_ADDR_SIZE];
	u8	RemoteName[BT_NAME_SIZE];
}
PACKED;

struct TBTHCIEventCommandComplete
{
	TBTHCIEventHeader	Header;

	u8	NumHCICommandPackets;
	u16	CommandOpCode;
	u8	Status;				// normally part of ReturnParameter[]
	u8	ReturnParameter[0];
}
PACKED;

struct TBTHCIEventReadBDAddrComplete
{
	TBTHCIEventCommandComplete	Header;

	u8	BDAddr[BT_BD_ADDR_SIZE];
}
PACKED;

struct TBTHCIEventCommandStatus
{
	TBTHCIEventHeader	Header;

	u8	Status;
	u8	NumHCICommandPackets;
	u16	CommandOpCode;
}
PACKED;

#endif
