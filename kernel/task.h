#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include "MemoryMap.h"
#include "stdint.h"

#define NOT_ENOUGH_TASK_NUM 0xFFFFFFFF
#define USR_TASK_STACK_SIZE 0x100000 // 1mb
#define MAX_TASK_NUM        (TASK_STACK_SIZE / USR_TASK_STACK_SIZE)

typedef struct KernelTaskContext_t
{
    uint32_t spsr;
    uint32_t r0_r12[13];
    uint32_t pc;
}KernelTaskContext_t;
// abstract contect - backup program status register, general register

typedef struct KernelTcb_t
{
    /* KernelTaskContext_t ctx; */
    uint32_t sp;
    uint8_t* stack_base;
    uint32_t priority;
}KernelTcb_t;
// stack infomation - sp is in general register stack pointer. stack_base is indivisual stack addr

typedef void(*KernelTaskFunc_t)(void);

void kernel_task_init(void); // init kernel task function
uint32_t kernel_task_create(KernelTaskFunc_t startFunc, uint32_t priority); // register kernel task

#endif