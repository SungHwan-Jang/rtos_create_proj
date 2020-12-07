#include "stdint.h"
#include "stdbool.h"
#include "hal_timer.h"
#include "stdlib.h"

void delay(uint32_t ms){
#if 0 
    uint32_t goal = hal_timer_get_1ms_counter() + ms;

    while(goal != hal_timer_get_1ms_counter());
#else

    uint32_t curCounter = hal_timer_get_1ms_counter();
    while(curCounter + ms != hal_timer_get_1ms_counter());

#endif
}
