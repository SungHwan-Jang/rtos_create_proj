#include "task.h"
#include "stdint.h"
#include "stdbool.h"

#include "ARMv7AR.h"

static KernelTcb_t sTask_list[MAX_TASK_NUM];
static KernelTcb_t* sCurrent_tcb;
static KernelTcb_t* sNext_tcb;

static uint32_t sAllocated_tcb_index;
static uint32_t sCurrent_tcb_index;

static KernelTcb_t* scheduler_round_robin_algorithm(void);
static KernelTcb_t* scheduler_priority_algorithm(void);
static __attribute__ ((naked)) void kernel_task_context_switching(void);

void kernel_task_init(void){

    sAllocated_tcb_index = 0;
    sCurrent_tcb_index = 0;

    for(uint32_t i=0; i<MAX_TASK_NUM; i++){

        sTask_list[i].stack_base = (uint8_t*)(TASK_STACK_START + i * USR_TASK_STACK_SIZE);
        sTask_list[i].sp = (uint32_t)sTask_list[i].stack_base + USR_TASK_STACK_SIZE - 4;

        sTask_list[i].sp -= sizeof(KernelTaskContext_t);

        KernelTaskContext_t* ctx = (KernelTaskContext_t*)sTask_list[i].sp;
        ctx->pc = 0;
        ctx->spsr = ARM_MODE_BIT_SYS;
    }
    // [ctx] = [stack sp]
    // [ctx...]
    // [ctx sp] = [ctx]
    // [others..]
    // [stack base]
} // init kernel task function

uint32_t kernel_task_create(KernelTaskFunc_t startFunc, uint32_t priority){

    KernelTcb_t* new_tcb = &sTask_list[sAllocated_tcb_index++];

    if(sAllocated_tcb_index > MAX_TASK_NUM){

        return NOT_ENOUGH_TASK_NUM;
    }

    new_tcb->priority = priority;

    KernelTaskContext_t* ctx = (KernelTaskContext_t*)new_tcb->sp;
    ctx->pc = (uint32_t)startFunc;

    return (sAllocated_tcb_index - 1);
} //register kernel task



void kernel_task_scheduler(void){

    sCurrent_tcb = &sTask_list[sCurrent_tcb_index];
    sNext_tcb = scheduler_round_robin_algorithm();

    kernel_task_context_switching();
}

void kernel_task_start(void){

}

//if define __attribute__ ((naked)) , do not automatically generate stack backup/restore/return adding func related ASM . only inner code exist.
static __attribute__ ((naked)) void kernel_task_context_switching(void){

    __asm__ ("B save_context");
    __asm__ ("B restore_context"); // B ASM means that no change LR.
}


static __attribute__ ((naked)) void save_context(void)
{
    // save current task context into the current task stack
    __asm__ ("PUSH {lr}");
    __asm__ ("PUSH {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}");
    __asm__ ("MRS   r0, cpsr");
    __asm__ ("PUSH {r0}");
    // save current task stack pointer into the current TCB
    __asm__ ("LDR   r0, =sCurrent_tcb");
    __asm__ ("LDR   r0, [r0]");
    __asm__ ("STMIA r0!, {sp}");
}

static __attribute__ ((naked)) void restore_context(void)
{
    // restore next task stack pointer from the next TCB
    __asm__ ("LDR   r0, =sNext_tcb");
    __asm__ ("LDR   r0, [r0]");
    __asm__ ("LDMIA r0!, {sp}");
    // restore next task context from the next task stack
    __asm__ ("POP  {r0}");
    __asm__ ("MSR   cpsr, r0");
    __asm__ ("POP  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}");
    __asm__ ("POP  {pc}");
}



static KernelTcb_t* scheduler_round_robin_algorithm(void){

    sCurrent_tcb_index++;
    sCurrent_tcb_index %= sAllocated_tcb_index;

    return &sTask_list[sCurrent_tcb_index];
}

static KernelTcb_t* scheduler_priority_algorithm(void){

#if 0

    for (uint32_t i = 0; i < sAllocated_tcb_index; i++){
        
        KernelTcb_t* pNextTcb = &sTask_list[i];

        if(pNextTcb != pNextTcb){

            // if(pNextTcb->priority <= sCurrent->priority){
                return pNextTcb;
            }

        }
    }

    return sCurrnet_tcb;

#endif
}