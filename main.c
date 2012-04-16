
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

uint8_t receive_packet();
void transmit_ack();

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

    // Set the transmitter address
    Radio_Set_Tx_Addr(trans_addr);  // or use the address manually informed by trans_addr.

    // print a message to UART to indicate that the program has started up
    uart_puts("Receive Station started\n\r");


    for(;;)
    {
        if (rxflag == 1)
        {

            if (!receive_packet())
            {

                rxflag = 0;
                transmit_ack();

            }

        }
    }

    return;
}

// Receive and print the received packet. This function is only for easy readability.
uint8_t receive_packet()
{

    rx_status = Radio_Receive(&packet); // Copy received packet to memory and store the result in rx_status.
    if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS) // Check if a packet is available.
    {

        if (packet.type != MESSAGE)
        {
            snprintf(output, sizeof(output), "Error: wrong packet type: %d. Should be %d\n\r", packet.type, MESSAGE);
            uart_puts(output);
        }

        // Print out the message, along with the message ID and sender address.
        snprintf(output, sizeof(output), "Message ID %d from 0x%.2X%.2X%.2X%.2X%.2X: '%s'\n\r",
                packet.payload.message.messageid,
                packet.payload.message.address[0],
                packet.payload.message.address[1],
                packet.payload.message.address[2],
                packet.payload.message.address[3],
                packet.payload.message.address[4],
                packet.payload.message.messagecontent);
        uart_puts(output);

        return 0;
    }
    else
    {
        return 1;
    }

}

// Transmit an ACK packet. This function is only for easy readability.
void transmit_ack()
{

    // Use the commented line below to set the transmit address to the one specified in
    // the received message packet. By default use the address already set.
    // Radio_Set_Tx_Addr(packet.payload.message.address);


    // Reply to the sender by sending an ACK packet, reusing the packet data structure.
    packet.type = ACK;
    // Se the ack message id:
    packet.payload.ack.messageid = 44;


    if (Radio_Transmit(&packet, RADIO_WAIT_FOR_TX) == RADIO_TX_MAX_RT)
    {
        // If the max retries was reached, the packet was not acknowledged.
        // This usually occurs if the receiver was not configured correctly or
        // if the sender didn't copy its address into the radio packet properly.
        snprintf(output, sizeof(output), "Could not reply to sender.\n\r");
        uart_puts(output);
    }
    else
    {
        // the transmission was completed successfully
        snprintf(output, sizeof(output), "ACK sent.\r\n");
        uart_puts(output);
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
