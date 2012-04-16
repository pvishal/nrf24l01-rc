
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

uint8_t station_addr[5] = { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 }; // Receiver address
uint8_t trans_addr[5] = { 0x98, 0x76, 0x54, 0x32, 0x10 }; // Transmitter address

radiopacket_t packet;


void transmit_message();

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
    uint16_t c;

    /* Initialize UART library by passing the baudrate */
    uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(BAUD_RATE, F_CPU) );

    sei();

    delay(10);

    Radio_Init();
    uart_puts("Device Ready!\r\n");

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, trans_addr, ENABLE);
    // configure radio transceiver settings.
    Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);

    Radio_Set_Tx_Addr(station_addr);

    delay(100);

    uart_puts("Transmit Station started. Press any character.\n\r");


    for(;;)
    {
        c = uart_getc();

        // The rxflag is set by radio_rxhandler function below indicating that a
        // new packet is ready to be read.
        if (rxflag == 1)
        {
            if (Radio_Receive(&packet) != RADIO_RX_MORE_PACKETS) // Receive packet.
            {
                // if there are no more packets on the radio, clear the receive flag;
                // otherwise, we want to handle the next packet on the next loop iteration.
                rxflag = 0;
            }
            if (packet.type == ACK)
            {
                uart_puts("ACK received.\r\n");
            }
        }

        if ( c & UART_NO_DATA )
        {
            /* no data available from UART do nothing */
        }
        else
        {
            // new data available from UART. Transmit a packet
            transmit_message();
        }
    }

}


//Transmit a character array. This function is only for readability purposes.
void transmit_message()
{

    // load up the packet contents
    packet.type = MESSAGE;
    memcpy(packet.payload.message.address, trans_addr, RADIO_ADDRESS_LENGTH);
    packet.payload.message.messageid = 55;
    snprintf((char*)packet.payload.message.messagecontent, sizeof(packet.payload.message.messagecontent), "Test message.");

    if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT) // Transmitt packet.
    {
        uart_puts("Data not transmitted. Max retry.");
    }
    else // Transmitted succesfully.
    {
        uart_puts("Data transmitted.\r\n");
    }

    return;

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
