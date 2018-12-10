MBluetooth::MBluetooth (CInterruptSystem *mInterrupt)
: p_mInterrupt (mInterrupt)
{
    s_pThis = this;
    p_mBluetooth = 0;
}

MBluetooth::~MBluetooth (void)
{
    s_pThis = 0;
    p_mTimer = 0;
    p_mBluetooth = 0;
    p_mDWHCI = 0;
    p_mFileSystem = 0;
}

boolean MBluetooth::Initialize (void)
{

}

            else if (!strcmp(linhacomando,"INITBT") && iy == 6)
            {
                if (p_mBluetooth == 0)
                {
                    p_mBluetooth = new CBTSubSystem(p_mInterrupt);
                    p_mBluetooth->Initialize ();
                    statusUartInit = 0;
                }
            }
            else if (!strcmp(linhacomando,"LOADBT") && iy == 6) 
            {
//                loadbt();
                ix = 255;
            }
            else if (!strcmp(linhacomando,"SENDBT") && iy == 6) 
            {
                // A definir
                ix = 255;
            }
            else if ((!strcmp(linhacomando,"FINDBT") || !strcmp(linhacomando,"CONNBT")) && iy == 6) 
            {
                p_mScrTft->printf("Searching Bluetooth Devices...\n");

                p_mScrTft->printf("Inquiry is running for %u seconds\n", INQUIRY_SECONDS);

                CBTInquiryResults *pInquiryResults = p_mBluetooth->Inquiry (INQUIRY_SECONDS, (!strcmp(linhacomando,"CONNBT") ? INQUIRY_LIMITED_DISCOVERY : INQUIRY_FIND_DEVICES));

                if (pInquiryResults == 0)
                {
                    p_mScrTft->printf("Inquiry failed\n");
                }

                p_mScrTft->printf("Inquiry complete, %u device(s) found\n", pInquiryResults->GetCount ());

                if (pInquiryResults->GetCount () > 0)
                {
                    p_mScrTft->printf("# BD Address        Class  Name\n");

                    for (unsigned nDevice = 0; nDevice < pInquiryResults->GetCount (); nDevice++)
                    {
                        const u8 *pBDAddress = pInquiryResults->GetBDAddress (nDevice);
                        assert (pBDAddress != 0);
                        
                        const u8 *pClassOfDevice = pInquiryResults->GetClassOfDevice (nDevice);
                        assert (pClassOfDevice != 0);
                        
                        p_mScrTft->printf("%u %02X:%02X:%02X:%02X:%02X:%02X %02X%02X%02X %s\n",
                                nDevice+1,
                                (unsigned) pBDAddress[5],
                                (unsigned) pBDAddress[4],
                                (unsigned) pBDAddress[3],
                                (unsigned) pBDAddress[2],
                                (unsigned) pBDAddress[1],
                                (unsigned) pBDAddress[0],
                                (unsigned) pClassOfDevice[2],
                                (unsigned) pClassOfDevice[1],
                                (unsigned) pClassOfDevice[0],
                                pInquiryResults->GetRemoteName (nDevice));
                    }
                }

                delete pInquiryResults;

                ix = 255;
            }
