#ifndef __MBluetooth__
#define __MBluetooth__

class MBluetooth
{
public:
    MBluetooth (CInterruptSystem *mInterrupt);
    ~MBluetooth (void);

	boolean Initialize (void);

private
	CBTSubSystem *p_mBluetooth;

    unsigned char pBuffer[255];
}

#endif
