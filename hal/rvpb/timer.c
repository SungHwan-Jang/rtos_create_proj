#include "stdint.h"
#include "timer.h"
#include "hal_timer.h"
#include "hal_interrupt.h"

extern volatile Timer_t* Timer;

static void interrupt_handler(void);

static uint32_t internal_1ms_counter;

void hal_timer_init(void){

    //interface reset
    Timer->timerxcontrol.bits.TimerEn = 0;
    Timer->timerxcontrol.bits.TimerMode = 0;
    Timer->timerxcontrol.bits.OneShot = 0;
    Timer->timerxcontrol.bits.TimerSize = 0;
    Timer->timerxcontrol.bits.OneShot = 0;
    Timer->timerxcontrol.bits.IntEnable = 1;
    Timer->timerxload = 0;
    Timer->timerxvalue = 0xFFFFFFFF;

    // set periodic mode
    Timer->timerxcontrol.bits.TimerMode = TIMER_PERIOIC;
    Timer->timerxcontrol.bits.TimerSize = TIMER_32BIT_COUNTER;
    Timer->timerxcontrol.bits.OneShot = 0;
    Timer->timerxcontrol.bits.TimerPre = 0;
    Timer->timerxcontrol.bits.IntEnable = 1;

    //clk sources are 1Mhz or 32.768khz
    // 0x10001000 is sysctrl0 register. it have which clk source info. 0 is 1Mhz

    uint32_t interval_1ms = TIMER_1MHZ_1SEC_INTERVAL / 1000;

    Timer->timerxload = interval_1ms;
    Timer->timerxcontrol.bits.TimerEn = 1;

    internal_1ms_counter = 0;

    // register timer interrupt handler
    hal_interrupt_enable(TIMER_INTERRUPT);
    hal_interrupt_register_handler(interrupt_handler, TIMER_INTERRUPT);
}

uint32_t hal_timer_get_1ms_counter(void){
    return internal_1ms_counter;
}

static void interrupt_handler(void){

    internal_1ms_counter++;
    Timer->timerxintclr = 1; // must clear interrupt signal. if not, timer hw continues to signal the interrupt.
}

