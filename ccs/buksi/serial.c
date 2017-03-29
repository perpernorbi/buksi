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
#define RECEIVE_BUFFER_SIZE 10
static char receive_buffer[RECEIVE_BUFFER_SIZE];
static const char* receive_readptr;
static char* receive_writeptr;

#define SEND_BUFFER_SIZE 10
static char send_buffer[SEND_BUFFER_SIZE];
static const char* send_readptr;
static char* send_writeptr;

void serial_initialize()
{
	receive_readptr = receive_buffer-1;
	receive_writeptr = receive_buffer;

	send_readptr = send_buffer-1;
	send_writeptr = send_buffer;
}

void serial_receiveByte(char data)
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	(*receive_writeptr) = data;
	++receive_writeptr;
	if (receive_writeptr - receive_buffer > RECEIVE_BUFFER_SIZE) halt();
	__bis_SR_register(gie);
}

void serial_sendChar(char data)
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
//		UCA0TXBUF = data;
	(*send_writeptr) = data;
	++send_writeptr;
	if (send_writeptr - send_buffer > RECEIVE_BUFFER_SIZE) halt();
	if (send_writeptr == (send_buffer + 1)) UCA0TXBUF = (*serial_getNextCharToSend());
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
		__bis_SR_register(gie);
		return receive_readptr;
	} else {
		__bis_SR_register(gie);
		return NULL;
	}
}

const char * serial_getNextCharToSend()
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	++send_readptr;
	if (send_readptr == send_writeptr) {
		send_readptr = send_buffer-1;
		send_writeptr = send_buffer;
		__bis_SR_register(gie);
		return NULL;
	}
	__bis_SR_register(gie);
	return send_readptr;
}
