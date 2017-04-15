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


#define FRAME_SIZE 3
#define RECEIVE_BUFFER_SIZE 100
static char receive_buffer[RECEIVE_BUFFER_SIZE];
static const char* receive_readptr;
static char* receive_writeptr;

#define SEND_BUFFER_SIZE 10
static char send_buffer[SEND_BUFFER_SIZE];
static const char* send_readptr;
static char* send_writeptr;

static char * const init_send_ptr = send_buffer - 1;

typedef enum {
	receiver_state_control = 0,
	receiver_state_data1 = 1,
	receiver_state_data2 = 2
} receiver_state;

receiver_state receiver_state_machine = receiver_state_control;

inline void initialize_sendptrs()
{
	send_readptr = init_send_ptr;
	send_writeptr = init_send_ptr;
}

void serial_initialize()
{
	receive_readptr = receive_buffer-1;
	receive_writeptr = receive_buffer;
	initialize_sendptrs();
}

void serial_receiveByte(char data)
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	if ((receiver_state_machine == receiver_state_control) && ((data & 0x80) != 0x80)) return;
	if (receiver_state_machine == receiver_state_data2) receiver_state_machine = receiver_state_control;
	else ++receiver_state_machine;
	(*receive_writeptr) = data;
	++receive_writeptr;
	__bis_SR_register(gie);
	if (receive_writeptr - receive_buffer > RECEIVE_BUFFER_SIZE) halt();
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
		if (send_writeptr - send_buffer >= RECEIVE_BUFFER_SIZE) halt();
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
		if (send_writeptr - send_buffer >= RECEIVE_BUFFER_SIZE) halt();
	}
	__bis_SR_register(gie);
}

const char * serial_getNextFrame()
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	++receive_readptr;
	while (((*receive_readptr) & 0x80 != 0x80) && (receive_readptr != receive_writeptr)) ++receive_readptr;
	if (receive_readptr == receive_writeptr) {
		receive_readptr = receive_buffer-1;
		receive_writeptr = receive_buffer;
		__bis_SR_register(GIE);
		return NULL;
	}
	if (receive_readptr + FRAME_SIZE <= receive_writeptr) {
		const char * retval = receive_readptr;
		receive_readptr += FRAME_SIZE - 1;
		__bis_SR_register(gie);
		return retval;
	} else {
		--receive_readptr;
		__bis_SR_register(gie);
		return NULL;
	}
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
