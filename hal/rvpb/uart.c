#include "stdint.h"
#include "uart.h"
#include "hal_uart.h"
#include "hal_interrupt.h"

#define _UN_OPTIMIZED_      0
#define _OPTIMIZED_LEV_1_      0
#define _OPTIMIZED_LEV_2_      1

extern volatile PL011_t* uart;

// test echo function
static void interrupt_handler(void){
    uint8_t ch = hal_uart_get_char();
    hal_uart_put_char(ch);

    //if(ch == ' ')
}

void hal_uart_init(void){
    //Enable uart
    uart->uartcr.bits.UARTEN = 0; //uart off
    uart->uartcr.bits.TXE = 1; // uart output on
    uart->uartcr.bits.RXE = 1; // uart input on
    uart->uartcr.bits.UARTEN = 1; //uart on

    // enable input interrupt
    uart->uartimsc.bits.RXIM = 1;

    // register uart interrupt handler
    hal_interrupt_enable(UART_INTERRUPT0);
    hal_interrupt_register_handler(interrupt_handler, UART_INTERRUPT0);
}

void hal_uart_put_char(uint8_t ch){
    
    while (uart->uartfr.bits.TXFF); // output buffer check. if 0, output buffer empty.
    
    uart->uartdr.all = (ch & 0xFF);
}

uint8_t hal_uart_get_char(void)
{
    uint8_t data;

    while (uart->uartfr.bits.RXFE)
        ;

#if _UN_OPTIMIZED_  // no optimized code
    //check for an error flag
    if (uart->uartdr.bits.BE ||
        uart->uartdr.bits.FE ||
        uart->uartdr.bits.OE ||
        uart->uartdr.bits.PE)
    {
        //clear the error
        uart->uartrsr.bits.BE = 1;
        uart->uartrsr.bits.FE = 1;
        uart->uartrsr.bits.OE = 1;
        uart->uartrsr.bits.PE = 1;

        return 0;
    }

    data = uart->uartdr.bits.DATA;

    return data;

#elif _OPTIMIZED_LEV_1_

    if(uart->uartdr.all & 0xFFFFFF00){
        uart->uartrsr.all = 0xFF;
        return 0;
    }

    data = uart->uartdr.bits.DATA;

    return data;

#elif _OPTIMIZED_LEV_2_

    data = uart->uartdr.all;

    // Check for an error flag
    if (data & 0xFFFFFF00)
    {
        // Clear the error
        uart->uartrsr.all = 0xFF;
        return 0;
    }

    return (uint8_t)(data & 0xFF);

#endif
    
}