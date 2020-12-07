/*
    if no handler.c code,
    arm cpu can not execute any hal interrupt handler fptr.
    hal_interrupt_init() only transfer interrupt number info to arm core exception vector
    so, make exception vector table and connect appropriate interrupt handler.
*/

#include "stdbool.h"
#include "stdint.h"
#include "hal_interrupt.h"

__attribute__ ((interrupt("IRQ"))) void IRQ_handler(void){
    hal_interrupt_run_handler();
}

__attribute__ ((interrupt("FIQ"))) void FIQ_handler(void){
    while (true); // dummy
}
