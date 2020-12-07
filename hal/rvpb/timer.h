/*
 * Timer.h
 *
 *  Created on: Sep 27, 2018
 *      Author: yiman
 */

#ifndef HAL_RVPB_TIMER_H_
#define HAL_RVPB_TIMER_H_

#include "stdint.h"

typedef union TimerXControl_t
{
    uint32_t all;
    struct {
        uint32_t OneShot:1;     //0 - if 1 one time interrupt
        uint32_t TimerSize:1;   //1 - timerxload / timerxvalue size setting 0 is 16bit 1 is 32 bit
        uint32_t TimerPre:2;    //3:2 - frequency demultiplier - 1 or 16 or 256clk decrease timer 1 value
        uint32_t Reserved0:1;   //4 
        uint32_t IntEnable:1;   //5 - interrupt on/off
        uint32_t TimerMode:1;   //6 - decide whether to use (timerxload) or not. if 0, free running mode 0xFFFFFFF ~ 0 interrupt occur / if 1 use timerxload init value. (periodic mode)
        uint32_t TimerEn:1;     //7 - on/off timer hw
        uint32_t Reserved1:24;  //31:8
    } bits;
} TimerXControl_t;

typedef union TimerXRIS_t
{
    uint32_t all;
    struct {
        uint32_t TimerXRIS:1;   //0
        uint32_t Reserved:31;   //31:1
    } bits;
} TimerXRIS_t;

typedef union TimerXMIS_t
{
    uint32_t all;
    struct {
        uint32_t TimerXMIS:1;   //0
        uint32_t Reserved:31;   //31:1
    } bits;
} TimerXMIS_t;

typedef struct Timer_t
{
    uint32_t        timerxload;     // 0x00 // timer goal value
    uint32_t        timerxvalue;    // 0x04 // decrementing reg (current timer val - load timerxload and decrenment, if 0 interrupt occur)
    TimerXControl_t timerxcontrol;  // 0x08 // (timer hw setting)
    uint32_t        timerxintclr;   // 0x0C // (reg notify complete of interrupt processing to timer hw)
    TimerXRIS_t     timerxris;      // 0x10
    TimerXMIS_t     timerxmis;      // 0x14
    uint32_t        timerxbgload;   // 0x18
} Timer_t;

#define TIMER_CPU_BASE  0x10011000
#define TIMER_INTERRUPT 36

#define TIMER_FREERUNNING   0
#define TIMER_PERIOIC       1

#define TIMER_16BIT_COUNTER 0
#define TIMER_32BIT_COUNTER 1

#define TIMER_10HZ_INTERVAL       (32768 * 4)
#define TIMER_1MHZ_1SEC_INTERVAL       (1024 * 1024)

#endif /* HAL_RVPB_TIMER_H_ */