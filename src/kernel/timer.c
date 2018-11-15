#include <common/stdlib.h>
#include <common/stdio.h>
#include <drivers/bcm2835min.h>
#include <kernel/interrupts.h>
#include "common/mylib.h"
#include "kernel/timer.h"
#include "drivers/rpi-usb-api.h"     

//#define __USE_USB__

static timer_registers_t * timer_regs;

static void timer_irq_handler(void) 
{
	#ifdef __USE_USB__
	    UsbCheckForChange();
	#endif
	    
    timer_set(1000); // 1mS
}

static void timer_irq_clearer(void) 
{
    timer_regs->control.timer1_matched = 1;
}

void TIMER_EN_INT(void)
{
    register_irq_handler(SYSTEM_TIMER_1, timer_irq_handler, timer_irq_clearer);
}

void TIMER_INIT(void)
{
    timer_regs = (timer_registers_t *) SYSTEM_TIMER_BASE;
}

void timer_set(uint32_t usecs) 
{
  timer_regs->timer1 = timer_regs->counter_low + usecs;
}

/*-[timer_getTickCount64]---------------------------------------------------}
. Get 1Mhz ARM system timer tick count in full 64 bit.
. The timer read is as per the Broadcom specification of two 32bit reads
. RETURN: tickcount value as an unsigned 64bit value in microseconds (usec)
. 30Jun17 LdB
.--------------------------------------------------------------------------*/
uint64_t timer_getTickCount(void)
{
	uint64_t resVal;
	uint32_t lowCount;
	do {
		resVal = timer_regs->counter_high; 								// Read Arm system timer high count
		lowCount = timer_regs->counter_low;							// Read Arm system timer low count
	} while (resVal != (uint64_t)timer_regs->counter_high);				// Check hi counter hasn't rolled in that time
	resVal = (uint64_t)resVal << 32 | lowCount;						// Join the 32 bit values to a full 64 bit
	return(resVal);													// Return the uint64_t timer tick count
}

/*-[timer_Wait]-------------------------------------------------------------}
. This will simply wait the requested number of microseconds before return.
. 02Jul17 LdB
.--------------------------------------------------------------------------*/
void timer_wait (uint64_t us) 
{
	us += timer_getTickCount();									// Add current tickcount onto delay
	while (timer_getTickCount() < us) {};							// Loop on timeout function until timeout
}
