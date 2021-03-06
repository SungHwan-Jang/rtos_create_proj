#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "hal_uart.h"
#include "hal_interrupt.h"
#include "hal_timer.h"
#include "kernel.h"

void user_task0(void);
void user_task1(void);


static void kernel_init(void);
static void hw_init(void);
static void printf_test(void);
static void timer_test(void);

void main(void)
{
    hw_init();

    uint32_t i = 100;
    while(i--){
        hal_uart_put_char('N');
    }

    hal_uart_put_char('\n');

    putstr("hello world!!\n");

    printf_test();
    
    //timer_test();
    kernel_init();

    while(true);

#if 0
    i = 100;

    while(i--){
        uint8_t ch = hal_uart_get_char();
        hal_uart_put_char(ch);
    }
#endif
}

static void hw_init(void){
    hal_interrupt_init();
    hal_uart_init();
    hal_timer_init();
}

static void printf_test(void)
{
    char *str = "printf pointer test";
    char *nullptr = 0;
    uint32_t i = 5;
    uint32_t *sysctrl0 = (uint32_t *)0x10001000;

    trace("%s\n", "Hello printf");
    trace("output string pointer: %s\n", str);
    trace("%s is null pointer, %u number\n", nullptr, 10);
    trace("%u = 5\n", i);
    trace("dec=%u hex=%x\n", 0xff, 0xff);
    trace("SYSCTRL0 0x%x (0 is 1Mhz, 1 is 32.768kHz)", *sysctrl0);
}

static void timer_test(void){
    uint32_t i =0;
    for(i=0; i<5; i++){
        trace("currnet count : %u\n", hal_timer_get_1ms_counter());
        delay(1000);
    }
}

void user_task0(void){

    uint32_t local = 0;

   
    while(true){
        delay(1000);
        trace("user task0 \n");
        trace("user task #0 SP=0x%x\n", &local);
        kernel_yield();
    }
}

void user_task1(void){

    uint32_t local = 0;

    while(true){
        delay(1000);
        trace("user task1 \n");
        trace("user task #1 SP=0x%x\n", &local);
        kernel_yield();
    }
}


static void kernel_init(void){
    uint32_t taskId;

    kernel_task_init();

    trace("kernel task init\n");

    taskId = kernel_task_create(user_task0, 0);

    if(taskId == NOT_ENOUGH_TASK_NUM){
        trace("task0 creation fail\n");
    }

    taskId = kernel_task_create(user_task1, 1);

    if(taskId == NOT_ENOUGH_TASK_NUM){
        trace("task0 creation fail\n");
    }

    kernel_start();
}