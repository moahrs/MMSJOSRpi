#ifndef _circle_kernel_h
#define _circle_kernel_h

#include <circlelib/circlelib.h>

#define __USE_CIRCLE_BLUETOOTH__

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
    CLcdVdg             m_LcdVdg;
    #ifdef __USE_TFT_LCD__
        CScrTft             m_ScrTft;
    #else
        CScreenDevice       m_Screen;
    #endif
    CExceptionHandler   m_ExceptionHandler;
    CInterruptSystem    m_Interrupt;
    CTimer              m_Timer;
    CLogger             m_Logger;
    CDWHCIDevice        m_DWHCI;
    CScheduler          m_Scheduler;
    #ifdef __USE_CIRCLE_BLUETOOTH__
        CBTSubSystem        m_Bluetooth;
    #endif
    CFATFileSystem      m_FileSystem;
    CLcdTch             m_LcdTch;
    CMMSJOS             m_MMSJOS;
};

#endif
