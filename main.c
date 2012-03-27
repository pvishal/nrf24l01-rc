/*************************************************************************
Title:    example program for the Interrupt controlled UART library
Author:   Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:     $Id: test_uart.c,v 1.4 2005/07/10 11:46:30 Peter Exp $
*************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "uart.h"

#define BAUD_RATE   115200

void delay(unsigned char count)
{
    unsigned char i = 0;
    for(;i<count;i++)
    {
        _delay_ms(10);
    }
}

int main(void)
{

    unsigned int c;

    /*
     * Set Pin 13 (PD7) of Arduino  as output for the LED
     */

    DDRB |= _BV(PB5);

    /*
     *  Initialize UART library by passing the baudrate
     */
    uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(BAUD_RATE, F_CPU) );

    /*
     * Now enable interrupts
     */
    sei();

    /*
     *  Transmit string to UART
     *  The string is buffered by the uart library in a circular buffer
     *  and one character at a time is transmitted to the UART using interrupts.
     *  uart_puts() blocks if it can not write the whole string to the circular
     *  buffer
     */
    uart_puts("Device Ready!\r\n");

    for(;;)
    {
        c = uart_getc();
        if ( c & UART_NO_DATA )
        {
            /*
             * no data available from UART
             * do nothing.
             */
        }
        else
        {
            /*
             * new data available from UART
             * send it back
             */
            uart_putc( (unsigned char)c );
            PORTB ^= _BV(PB5);
        }
    }

}
