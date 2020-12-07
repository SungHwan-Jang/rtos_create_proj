
#include "stdint.h"
#include "memio.h"
#include "interrupt.h"
#include "hal_interrupt.h"
#include "armcpu.h"

extern volatile GicCput_t* GicCpu;
extern volatile GicDist_t* GicDist;

static InterruptHandler_fptr sHandlers[INTERRUPT_HANDLER_NUM];

void hal_interrupt_init(void){

    GicCpu->cpucontrol.bits.Enable = 1;
    GicCpu->prioritymask.bits.Prioritymask = GIC_PRIORITY_MASK_NONE; //enable 0x00 ~ 0x0E priority interrupt
    GicDist->distributorctrl.bits.Enable = 1;

    for(uint32_t i=0; i< INTERRUPT_HANDLER_NUM; i++){
        sHandlers[i] = NULL;
    }

    enable_irq();
}

void hal_interrupt_enable(uint32_t interrupt_num){
    
    if((interrupt_num < GIC_IRQ_START) || (interrupt_num > GIC_IRQ_END)){
        return; // err
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if(bit_num < GIC_IRQ_START){
        SET_BIT(GicDist->setenable1, bit_num);
    }
    else{
        bit_num -= GIC_IRQ_START;
        SET_BIT(GicDist->setenable2, bit_num);
    }
}

void hal_interrupt_disable(uint32_t interrupt_num){
    if((interrupt_num < GIC_IRQ_START) || (interrupt_num > GIC_IRQ_END)){
        return; // err
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if(bit_num < GIC_IRQ_START){
        CLR_BIT(GicDist->setenable1, bit_num);
    }
    else{
        bit_num -= GIC_IRQ_START;
        CLR_BIT(GicDist->setenable2, bit_num);
    }
}

void hal_interrupt_register_handler(InterruptHandler_fptr handler, uint32_t interrupt_num){
    sHandlers[interrupt_num] = handler;
}

void hal_interrupt_run_handler(void){
    uint32_t interrupt_num = GicCpu->interruptack.bits.InterruptID;

    if(sHandlers[interrupt_num] != NULL){
        sHandlers[interrupt_num]();
    }

    GicCpu->endofinterrupt.bits.InterruptID = interrupt_num;
}
