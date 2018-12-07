//
// btlogicallayer.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015  R. Stange <rsta2@o2online.de>
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
#include <circle/bt/btlogicallayer.h>
#include <circle/bt/bluetooth.h>
#include <circle/bt/btglobal.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <assert.h>

CBTLogicalLayer::CBTLogicalLayer (CBTHCILayer *pHCILayer)
:	m_pHCILayer (pHCILayer),
	m_pInquiryResults (0),
	m_pBuffer (0)
{
}

CBTLogicalLayer::~CBTLogicalLayer (void)
{
	assert (m_pInquiryResults == 0);

	delete [] m_pBuffer;
	m_pBuffer = 0;

	m_pHCILayer = 0;
}

boolean CBTLogicalLayer::Initialize (void)
{
	m_pBuffer = new u8[BT_MAX_DATA_SIZE];
	assert (m_pBuffer != 0);
	mConnComplete = 0x00;
	mBtHandleActive = 0x00;

	return TRUE;
}

void CBTLogicalLayer::Process (void)
{
	assert (m_pHCILayer != 0);
	assert (m_pBuffer != 0);

	unsigned nLength;
	while (m_pHCILayer->ReceiveLinkEvent (m_pBuffer, &nLength))
	{
		assert (nLength >= sizeof (TBTHCIEventHeader));
		TBTHCIEventHeader *pHeader = (TBTHCIEventHeader *) m_pBuffer;

		switch (pHeader->EventCode)
		{
		case EVENT_CODE_INQUIRY_RESULT: {
			assert (nLength >= sizeof (TBTHCIEventInquiryResult));
			TBTHCIEventInquiryResult *pEvent = (TBTHCIEventInquiryResult *) pHeader;
			assert (nLength >=   sizeof (TBTHCIEventInquiryResult)
					   + pEvent->NumResponses * INQUIRY_RESP_SIZE);

			assert (m_pInquiryResults != 0);
			m_pInquiryResults->AddInquiryResult (pEvent);
			} break;

		case EVENT_CODE_INQUIRY_COMPLETE: {
			assert (nLength >= sizeof (TBTHCIEventInquiryComplete));
			TBTHCIEventInquiryComplete *pEvent = (TBTHCIEventInquiryComplete *) pHeader;

			if (pEvent->Status != BT_STATUS_SUCCESS)
			{
				delete m_pInquiryResults;
				m_pInquiryResults = 0;

				m_Event.Set ();

				break;
			}

			m_nNameRequestsPending = m_pInquiryResults->GetCount ();
			if (m_nNameRequestsPending == 0)
			{
				m_Event.Set ();

				break;
			}

			assert (m_pInquiryResults != 0);
			for (unsigned nResponse = 0; nResponse < m_pInquiryResults->GetCount (); nResponse++)
			{
				TBTHCIRemoteNameRequestCommand Cmd;
				Cmd.Header.OpCode = OP_CODE_REMOTE_NAME_REQUEST;
				Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
				memcpy (Cmd.BDAddr, m_pInquiryResults->GetBDAddress (nResponse), BT_BD_ADDR_SIZE);
				Cmd.PageScanRepetitionMode = m_pInquiryResults->GetPageScanRepetitionMode (nResponse);
				Cmd.Reserved = 0;
				Cmd.ClockOffset = CLOCK_OFFSET_INVALID;
				m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);
			}
			} break;

		case EVENT_CODE_REMOTE_NAME_REQUEST_COMPLETE: {
			assert (nLength >= sizeof (TBTHCIEventRemoteNameRequestComplete));
			TBTHCIEventRemoteNameRequestComplete *pEvent = (TBTHCIEventRemoteNameRequestComplete *) pHeader;

			if (pEvent->Status == BT_STATUS_SUCCESS)
			{
				assert (m_pInquiryResults != 0);
				m_pInquiryResults->SetRemoteName (pEvent);
			}

			if (--m_nNameRequestsPending == 0)
			{
				m_Event.Set ();
			}
			} break;

		case EVENT_CODE_COMMAND_COMPLETE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Command Complete");

				assert (nLength >= sizeof (TBTHCIEventCommandComplete));
				TBTHCIEventCommandComplete *pEvent = (TBTHCIEventCommandComplete *) pHeader;

				if (pEvent->Status == BT_STATUS_SUCCESS)
				{
				}
				else
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
		} break;

		case EVENT_CODE_DISCONNECT: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Disconnect Received");

				assert (nLength >= sizeof (TBTHCIEventDisconnect));
				TBTHCIEventDisconnect *pEvent = (TBTHCIEventDisconnect *) pHeader;

				if (pEvent->Status == BT_STATUS_SUCCESS)
				{
					mBtHandleActive = 0;
				}
				else
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
		} break;

		case EVENT_CODE_AUTH_COMPLETE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Auth Complete");

				assert (nLength >= sizeof (TBTHCIEventAuthComplete));
				TBTHCIEventAuthComplete *pEvent = (TBTHCIEventAuthComplete *) pHeader;

				if (pEvent->Status == BT_STATUS_SUCCESS)
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Sucess. handle: %02x", pEvent->ConnHandle);
					mBtHandleActive = pEvent->ConnHandle;
				}
				else
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
		} break;

		case EVENT_CODE_CONNECT_REQUEST: {
				assert (nLength >= sizeof (TBTHCIEventConnectRequest));
				TBTHCIEventConnectRequest *pEvent = (TBTHCIEventConnectRequest *) pHeader;

				CLogger::Get ()->Write ("BT> ", LogNotice, "Connect Request from BDAddr: %02x:%02x:%02x:%02x:%02x:%02x ClassDev: %06x LinkTp: %02x", 
										(unsigned) pEvent->BDAddr[5],
										(unsigned) pEvent->BDAddr[4],
										(unsigned) pEvent->BDAddr[3],
										(unsigned) pEvent->BDAddr[2],
										(unsigned) pEvent->BDAddr[1],
										(unsigned) pEvent->BDAddr[0],
										pEvent->ClassOfDevice,
										pEvent->LinkType);

				if (pEvent->LinkType == 0x01)	// ACL
				{
					TBTHCIAcceptConnectionRequestCommand Cmd;
					Cmd.Header.OpCode = OP_HCI_ACCEPT_CONNECT_REQUEST;
					Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
					memcpy (Cmd.BDAddr, pEvent->BDAddr, BT_BD_ADDR_SIZE);
					Cmd.RemainSlave = 0x01;
					m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);
				}
				else	// SCO or eSCO
				{					
					TBTHCIAcceptSyncConnectionRequestCommand Cmd;
					Cmd.Header.OpCode = OP_HCI_ACCEPT_SYNC_CONNECT_REQUEST;
					Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
					memcpy (Cmd.BDAddr, pEvent->BDAddr, BT_BD_ADDR_SIZE);
					Cmd.Transmit_Bandwidth = 0x00001F40;
					Cmd.Receive_Bandwidth = 0x00001F40;
					Cmd.Max_Latency = 0xFFFF;
					Cmd.Voice_Setting = 0b0011000000;
					Cmd.Retransmission_Effort = 0xFF;
					Cmd.Packet_Type = 0x003F;
					m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);
				}
			} break;

		case EVENT_CODE_PIN_CODE_REQUEST: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Pin Code Request");

				assert (nLength >= sizeof (TBTHCIEventPinCodeRequest));
				TBTHCIEventPinCodeRequest *pEvent = (TBTHCIEventPinCodeRequest *) pHeader;

				if (mConnComplete)
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Pin Code Sent");

					TBTHCIPinCodeRequestReplyCommand Cmd;
					Cmd.Header.OpCode = OP_CODE_PIN_CODE_REQUEST_REPLY;
					Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
					memcpy (Cmd.BDAddr, pEvent->BDAddr, BT_BD_ADDR_SIZE);
					Cmd.PinCodeLength = 4;
					memset(Cmd.PinCode,0x00,16);
					Cmd.PinCode[3] = 0x34;
					Cmd.PinCode[2] = 0x33;
					Cmd.PinCode[1] = 0x32;
					Cmd.PinCode[0] = 0x31;
					m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);
				}
			} break;

		case EVENT_LINK_KEY_NOTIFICATION: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Link Key Notify");

				assert (nLength >= sizeof (TBTHCIEventLinkKeyNotification));
				TBTHCIEventLinkKeyNotification *pEvent = (TBTHCIEventLinkKeyNotification *) pHeader;

				CLogger::Get ()->Write ("BT> ", LogNotice, "Write Policy Sent");

				TBTHCIWriteLinkPolicySettingsCommand Cmd;
				Cmd.Header.OpCode = OP_HCI_WRITE_LINK_POLICY_SETTINGS;
				Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
				Cmd.ConnHandle = mBtHandleActive;
				Cmd.Link_Policy_Settings = 0x0000;
				m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);
			} break;

		case EVENT_ENCRYPTION_CHANGE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Encrypt Change");

				assert (nLength >= sizeof (TBTHCIEventEncryptionChange));
				TBTHCIEventEncryptionChange *pEvent = (TBTHCIEventEncryptionChange *) pHeader;

				if (pEvent->Status != BT_STATUS_SUCCESS)
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
			} break;

		case EVENT_CODE_SYNC_CONNECT_COMPLETE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Sync Connect Complete");

				assert (nLength >= sizeof (TBTHCIEventSyncConnectComplete));
				TBTHCIEventSyncConnectComplete *pEvent = (TBTHCIEventSyncConnectComplete *) pHeader;

				if (pEvent->Status == BT_STATUS_SUCCESS)
				{
					mConnComplete = 0x01;
					CLogger::Get ()->Write ("BT> ", LogNotice, "Handle: %04x BDAddr: %02x:%02x:%02x:%02x:%02x:%02x LinkTp: %02x", 
											pEvent->ConnHandle, 
											(unsigned) pEvent->BDAddr[5],
											(unsigned) pEvent->BDAddr[4],
											(unsigned) pEvent->BDAddr[3],
											(unsigned) pEvent->BDAddr[2],
											(unsigned) pEvent->BDAddr[1],
											(unsigned) pEvent->BDAddr[0],
											pEvent->LinkType);
				}
				else
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
		} break;

		case EVENT_CODE_MAX_SLOTS_CHANGE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Max Slots Change");

				assert (nLength >= sizeof (TBTHCIEventMaxSlotsChange));
				TBTHCIEventMaxSlotsChange *pEvent = (TBTHCIEventMaxSlotsChange *) pHeader;

				CLogger::Get ()->Write ("BT> ", LogNotice, "Handle: %04x LMPMaxSlots: %02x", 
										pEvent->ConnHandle, 
										pEvent->LMPMaxSlots);
		} break;

		case EVENT_CODE_CONNECT_COMPLETE: {
				CLogger::Get ()->Write ("BT> ", LogNotice, "Connect Complete");

				assert (nLength >= sizeof (TBTHCIEventConnectComplete));
				TBTHCIEventConnectComplete *pEvent = (TBTHCIEventConnectComplete *) pHeader;

				if (pEvent->Status == BT_STATUS_SUCCESS)
				{
					mConnComplete = 0x01;
					CLogger::Get ()->Write ("BT> ", LogNotice, "Handle: %04x BDAddr: %02x:%02x:%02x:%02x:%02x:%02x LinkTp: %02x EncrpEn: %02x", 
											pEvent->ConnHandle, 
											(unsigned) pEvent->BDAddr[5],
											(unsigned) pEvent->BDAddr[4],
											(unsigned) pEvent->BDAddr[3],
											(unsigned) pEvent->BDAddr[2],
											(unsigned) pEvent->BDAddr[1],
											(unsigned) pEvent->BDAddr[0],
											pEvent->LinkType,
											pEvent->EncryptEnabled);
				}
				else
				{
					CLogger::Get ()->Write ("BT> ", LogNotice, "Status: %04x", pEvent->Status);
				}
			} break;

		default:
			if (pHeader->EventCode != 0x00)
				CLogger::Get ()->Write ("BT> ", LogNotice, "%04X.",pHeader->EventCode);
			break;
		}
	}
}

CBTInquiryResults *CBTLogicalLayer::Inquiry (unsigned nSeconds, unsigned tType)
{
	assert (1 <= nSeconds && nSeconds <= 61);
	assert (m_pHCILayer != 0);

	assert (m_pInquiryResults == 0);
	m_pInquiryResults = new CBTInquiryResults;
	assert (m_pInquiryResults != 0);

	m_Event.Clear ();

	TBTHCIInquiryCommand Cmd;
	Cmd.Header.OpCode = OP_CODE_INQUIRY;
	Cmd.Header.ParameterTotalLength = PARM_TOTAL_LEN (Cmd);
	Cmd.LAP[0] = (tType == 0 ? INQUIRY_LAP_GIAC : INQUIRY_LAP_LIAC)       & 0xFF;
	Cmd.LAP[1] = (tType == 0 ? INQUIRY_LAP_GIAC : INQUIRY_LAP_LIAC) >> 8  & 0xFF;
	Cmd.LAP[2] = (tType == 0 ? INQUIRY_LAP_GIAC : INQUIRY_LAP_LIAC) >> 16 & 0xFF;
	Cmd.InquiryLength = INQUIRY_LENGTH (nSeconds);
	Cmd.NumResponses = INQUIRY_NUM_RESPONSES_UNLIMITED;
	m_pHCILayer->SendCommand (&Cmd, sizeof Cmd);

	m_Event.Wait ();

	CBTInquiryResults *pResult = m_pInquiryResults;
	m_pInquiryResults = 0;
	
	return pResult;
}
