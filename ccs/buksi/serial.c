/*
 * serial.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include "serial.h"
#include "halt.h"
#include <msp430.h>
#include <stddef.h>


static unsigned char receive_readindex;
static unsigned char receive_writeindex;

typedef enum {
	receiver_state_control = 0,
	receiver_state_data1 = 1,
	receiver_state_data2 = 2,
	receiver_state_crc = 3
} receive_state_machine;
#define FRAME_SIZE 3
#define RECEIVE_BUFFER_SIZE 4
#define RECEIVE_BUFFER_SIZE_MASK 0x03
static char receive_buffer[RECEIVE_BUFFER_SIZE][FRAME_SIZE];
static char receiver_crc;
static receive_state_machine receiver_state = receiver_state_control;

#define SEND_BUFFER_SIZE 10
static char send_buffer[SEND_BUFFER_SIZE];
static const char* send_readptr;
static char* send_writeptr;

static char * const init_send_ptr = send_buffer - 1;



inline void initialize_sendptrs()
{
	send_readptr = init_send_ptr;
	send_writeptr = init_send_ptr;
}

void serial_initialize()
{
	receive_readindex = 0;
	receive_writeindex = 0;
	receiver_crc=0;
	initialize_sendptrs();
}

void serial_receiveByte(char data)
{
//	unsigned short gie = _get_SR_register() & GIE;
	if ((receiver_state == receiver_state_control) && ((data & 0x80) != 0x80)) return;
//	__bic_SR_register(GIE);
	if (receiver_state != receiver_state_crc) {
		receive_buffer[receive_writeindex][receiver_state] = data;
		receiver_crc += data;
		++receiver_state;
	} else {
		if (receiver_crc == data) {
			receive_writeindex = (receive_writeindex + 1) & RECEIVE_BUFFER_SIZE_MASK;
			if (receive_readindex == receive_writeindex) halt();
		}
		receiver_state = receiver_state_control;
		receiver_crc = 0;
	}
//	__bis_SR_register(gie);
}

void serial_sendChar(char data)
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	if (send_writeptr == init_send_ptr) {
		++send_writeptr;
		UCA0TXBUF = data;
	} else {
		(*send_writeptr) = data;
		++send_writeptr;
		if (send_writeptr - send_buffer >= SEND_BUFFER_SIZE) halt();
		//if (send_writeptr == (send_buffer + 1)) UCA0TXBUF = (*serial_getNextCharToSend());
	}
	__bis_SR_register(gie);
}

void serial_sendString(const char* data)
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
//		UCA0TXBUF = data;
	if (send_writeptr == init_send_ptr) {
		++send_writeptr;
		UCA0TXBUF = data[0];
		++data;
	}
	while ((*data) != NULL) {
		(*send_writeptr) = (*data);
		++send_writeptr;
		++data;
		if (send_writeptr - send_buffer >= SEND_BUFFER_SIZE) halt();
	}
	__bis_SR_register(gie);
}

const char * serial_getNextFrame()
{
	if (receive_readindex == receive_writeindex) return NULL;
	const char* retval = receive_buffer[receive_readindex];
	receive_readindex = (receive_readindex + 1) & RECEIVE_BUFFER_SIZE_MASK;
	return retval;
}

const char * serial_getNextCharToSend()
{
	++send_readptr;
	if (send_readptr >= send_writeptr) {
		initialize_sendptrs();
		return NULL;
	}
	return send_readptr;
}

void serial_test()
{
	__bic_SR_register(GIE);
	serial_receiveByte(0x81);
	serial_receiveByte(0x40);
	serial_getNextFrame();
	serial_receiveByte(0x80);
	serial_getNextFrame();
	serial_getNextFrame();
}
