#include <kernel/interrupts.h>
#include <common/stdlib.h>
#include <drivers/lcd_tch.h>
#include <kernel/mmsjos.h>
#include <kernel/mmsjos_api.h>
#include <drivers/lcd_vdg_api.h>

static interrupt_registers_t * interrupt_regs;

static interrupt_handler_f handlers[NUM_IRQS];
static interrupt_clearer_f clearers[NUM_IRQS];

extern void move_exception_vector(void);
extern uint32_t exception_vector;

void interrupts_init(void) {
    interrupt_regs = (interrupt_registers_t *)INTERRUPTS_PENDING;
	bzero(handlers, sizeof(interrupt_handler_f) * NUM_IRQS);
	bzero(clearers, sizeof(interrupt_clearer_f) * NUM_IRQS);
	interrupt_regs->irq_basic_disable = 0xffffffff; // disable all interrupts
	interrupt_regs->irq_gpu_disable1 = 0xffffffff;
	interrupt_regs->irq_gpu_disable2 = 0xffffffff;
    move_exception_vector(); 
    ENABLE_INTERRUPTS();
}

/**
 * this function is going to be called by the processor.  Needs to check pending interrupts and execute handlers if one is registered
 */
void irq_handler(void) {
    int j; 

	for (j = 0; j < NUM_IRQS; j++) {
        // If the interrupt is pending and there is a handler, run the handler
        if (IRQ_IS_PENDING(interrupt_regs, j)  && (handlers[j] != 0)) {
			clearers[j]();
/*			ENABLE_INTERRUPTS();*/ // Se ativar essa merda, da pau e nao executa o handle... tem q ver por que... mas sem ele, funciona tudo certinho sem problemas
			handlers[j]();
			DISABLE_INTERRUPTS();
			return;
        }
    }
}

void __attribute__ ((interrupt ("ABORT"))) reset_handler(void) {
}
void __attribute__ ((interrupt ("ABORT"))) prefetch_abort_handler(void) {
}
void __attribute__ ((interrupt ("ABORT"))) data_abort_handler(void) {
}
void __attribute__ ((interrupt ("UNDEF"))) undefined_instruction_handler(void) {
}
void __attribute__ ((interrupt ("SWI"))) software_interrupt_handler(void) {
}
void __attribute__ ((interrupt ("FIQ"))) fast_irq_handler(void) {
}

void register_irq_handler(irq_number_t irq_num, interrupt_handler_f handler, interrupt_clearer_f clearer) {
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_basic_enable |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable2 |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable1 |= (1 << irq_pos);
    }
}

void unregister_irq_handler(irq_number_t irq_num) {
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        // Setting the disable bit clears the enabled bit
        interrupt_regs->irq_basic_disable |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        interrupt_regs->irq_gpu_disable2 |= (1 << irq_pos);
    }
    else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        interrupt_regs->irq_gpu_disable1 |= (1 << irq_pos);
    }
}

