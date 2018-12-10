#ifndef _circle_kernel_h
#define _circle_kernel_h

#include <circlelib/circlelib.h>

enum TShutdownMode
{
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CKernel
{
public:
    CKernel (void);
    ~CKernel (void);

    bool Initialize (void);
    
    TShutdownMode Run (void);


    // do not change this order
    CMemorySystem       m_Memory;
    CActLED             m_ActLED;
    CKernelOptions      m_Options;
    CDeviceNameService  m_DeviceNameService;
    CInterruptSystem    m_Interrupt;
    #ifdef __USE_TFT_LCD__
        CScrTft             m_ScrTft;
    #else
        CScreenDevice       m_Screen;
    #endif
    CExceptionHandler   m_ExceptionHandler;
    CTimer              m_Timer;
    CLogger             m_Logger;
    CDWHCIDevice        m_DWHCI;
    CScheduler          m_Scheduler;
    CBTSubSystem        m_Bluetooth;
    FATFS               m_FileSystem;
    CEMMCDevice         m_EMMC;
    CMMSJOS             m_MMSJOS;
};

#endif
