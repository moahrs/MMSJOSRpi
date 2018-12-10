#include <circlelib/circlelib.h>
#include <common/stdio.h>
#include <common/stdlib.h>
#include "common/mylib.h"
#include <drivers/uart.h>
#include <module_hm10.h>

MHm10 *MHm10::s_pThis = 0;
char statusHm10Init = 0;

MHm10::MHm10 (void)
{
    s_pThis = this;
    statusHm10Init = 0;
}

MHm10::~MHm10 (void)
{
    s_pThis = 0;
    statusHm10Init = 0;
}

boolean MHm10::Initialize (void)
{
    printf("Module BT Serial HM-10... ");
    puts("AT\r\n");
    CTimer::Get()->MsDelay (500);
    getst((char*)&pBuffer, 3);
    printf("%s\n",pBuffer);

    if (pBuffer[0] == 'O' && pBuffer[1] == 'K')
    {
        statusHm10Init = 1;
        return false;
    }
    else 
    {
        printf("\nModule Serial Bluetooth HM-10 Not Found\n");
        return true;
    }
}

int MHm10::Command(char* vCommand, char* vParam)
{
    if (!strcmp(vCommand,"HM10"))
    {
        if (statusHm10Init)
        {
            printf("Sending: %s\n",vParam);
            puts((char*)vParam);
            puts("\r\n");
            CTimer::Get()->MsDelay (500);
            getst((char*)&pBuffer, 65536);
            printf("Received: %s\n",pBuffer);
            printf("\n");
        }
    }
    else if (!strcmp(vCommand,"LOADSBT")) 
    {
        if (statusHm10Init)
            CMMSJOS::Get()->load232();
    }
    else if (!strcmp(vCommand,"SENDSBT")) 
    {
        // A definir
    }

    return 0;
}
