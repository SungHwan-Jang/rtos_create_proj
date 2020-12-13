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
static __attribute__ ((naked)) void save_context(void);
static __attribute__ ((naked)) void restore_context(void);

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
    
    sNext_tcb = &sTask_list[sCurrent_tcb_index];
    restore_context();
}

//if define __attribute__ ((naked)) , do not automatically generate stack backup/restore/return adding func related ASM . only inner code exist.
static __attribute__ ((naked)) void kernel_task_context_switching(void){
    __asm__ ("B save_context");
    __asm__ ("B restore_context"); // B ASM means that no change LR.
}

/* REGISTER MAP
 * R0-R12 : general purpose register
 * R13 : SP - save stack addr
 * R14 : LR - save return addr
 * R15 : PC - save next operation instruct addr
 */

/* MOV - transfer data (reg to reg)
 *     + if use constant val, have some restraint point. (ROR restraint, 8-bit val... check it, do not use MOV for constant value load cmd replace LDR cmd.)
 * 
 * LDR ( load to register ) - transfer data (mem to reg)
 *     + LDR Rn, =Value (try to load constant value to reg) / other ASM cmd use # for constant value
 *     + LDR Rd, [Rs] (load *Rs mem addr to Rd reg)
 * 
 * STR ( store to memory ) - transfer data (reg to mem)
 *     + LDR Rs, [Rd] (save Rs reg val to *Rd mem addr)
 */

/* simple ex) LDR r3, [r1,#0x1c] -> r3 = *(r1+0x1c); */

/* PUSH {r0-r12} -> 2 step ASM cmd
 * example > stack addr:0xD000D000 -> the number of reg = 13 so, 32bit-arch 13 * 4byte = need 52byte
 * step1. stack addr change [0xD000D000 - 0x34 (dec 52)] = 0xD000CFCC
 * step2. push r0 ~ r12
 * ex) 0xD000CFCC : r0 data
 *     0xD000CFD0 : r1 data
 *     0xD000CFD4 : r2 data
 *      ....
 *     0xD000CFFC : r12 data
 */

/* LDM r6, {r2-r3} 
 * [before] <reg> r6 : 0xC011AACC | <mem> 0xC011AACC : 10001000 / 0xC011AAD0 : E59B300C 
 * [after]  <reg> r2 : 10001000 / r3 : E59B300C |
 */

/* IA - after operation base register increase 1 word
 * IB - before operation base register increase 1 word
 * DA - after operation base register decrease 1 word
 * DB - before operation base register decrease 1 word
 * ! - retain addr pointerm after operation. if use LDM or STM, addr pointer decrease or increase and return base pointer, but use !, do not return addr pointer. 
 * 
 * LDMIA 
 */

/*
 * PSR ARM ASM CMD
 * 1. MRS - read cpsr or spsr register
 * MSR r0, cpsr -> r0 read cpsr register data.
 */

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