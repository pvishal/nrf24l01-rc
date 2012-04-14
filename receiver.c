
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "packet.h"
#include "radio.h"
#include "uart.h"

#define BAUD_RATE   115200

volatile uint8_t rxflag = 0;
volatile uint8_t txflag = 0;
char output[128];

uint8_t station_addr[5] = { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 }; // Receiver address
uint8_t trans_addr[5] = { 0x98, 0x76, 0x54, 0x32, 0x10 }; // Transmitter address
 
RADIO_RX_STATUS rx_status; 
 
radiopacket_t packet;


void delay(unsigned int count)
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

    /* Initialize UART library by passing the baudrate */
    uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(BAUD_RATE, F_CPU) );

    sei();

    delay(10);
    
    Radio_Init();
    uart_puts("Device Ready!\r\n");
    
 	// configure the receive settings for radio pipe 0
	Radio_Configure_Rx(RADIO_PIPE_0, station_addr, ENABLE);
 
	// configure radio transceiver settings.
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
    
    // print a message to UART to indicate that the program has started up
	uart_puts("STATION START\n\r");

    
    for(;;)
    {
        if (txflag == 1)
        {
            txflag = 0;
            uart_putc('t');
        }
        if (rxflag == 1)
        {
            rxflag = 0;
            uart_putc('r');
        }
    }
}

// The radio_rxhandler is called by the radio IRQ pin interrupt routine when RX_DR is read in STATUS register.
void radio_rxhandler(uint8_t pipe_number)
{
	rxflag = 1;
}

// The radio_txhandler is called by the radio IRQ pin interrupt routine when TX_DS is read in STATUS register.
void radio_txhandler(void)
{
	txflag = 1;
}