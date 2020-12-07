#include "stdint.h"
#include "uart.h"
#include "interrupt.h"
#include "timer.h"

// uart
volatile PL011_t* uart = (PL011_t*)UART_BASE_ADDRESS0;

//interrupt
volatile GicCput_t* GicCpu = (GicCput_t*)GIC_CPU_BASE;
volatile GicDist_t* GicDist = (GicDist_t*)GIC_DIST_BASE;

//timer
volatile Timer_t* Timer = (Timer_t*)TIMER_CPU_BASE;

