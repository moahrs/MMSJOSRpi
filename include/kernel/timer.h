#ifndef Timer_H
#define Timer_H

#define SYSTEM_TIMER_BASE (SYSTEM_TIMER_OFFSET + PERIPHERAL_BASE)

typedef struct {
    uint8_t timer0_matched: 1;
    uint8_t timer1_matched: 1;
    uint8_t timer2_matched: 1;
    uint8_t timer3_matched: 1;
    uint32_t reserved: 28;
} timer_control_reg_t;

typedef struct {
    timer_control_reg_t control;
    uint32_t counter_low;
    uint32_t counter_high;
    uint32_t timer0;
    uint32_t timer1;
    uint32_t timer2;
    uint32_t timer3;
} timer_registers_t;

extern void TIMER_EN_INT(void);
extern void TIMER_INIT(void);
extern void timer_set(uint32_t usecs);
extern uint64_t timer_getTickCount(void);
extern void timer_wait (uint64_t us);

#endif
