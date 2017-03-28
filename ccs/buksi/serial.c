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

void serial_initialize()
{
	receive_readptr = receive_buffer-1;
	receive_writeptr = receive_buffer;
}

void serial_receiveByte(char data)
{
	__bic_SR_register(GIE);
	(*receive_writeptr) = data;
	++receive_writeptr;
	if (receive_writeptr - receive_buffer > RECEIVE_BUFFER_SIZE) halt();
	__bis_SR_register(GIE);
}

const char * serial_getNextFrame()
{
	__bic_SR_register(GIE);
	++receive_readptr;
	while (((*receive_readptr) & 0x80 != 0x80) && (receive_readptr != receive_writeptr)) ++receive_readptr;
	if (receive_readptr == receive_writeptr) {
		serial_initialize();
		__bis_SR_register(GIE);
		return NULL;
	}
	if (receive_readptr + FRAME_SIZE <= receive_writeptr) {
		__bis_SR_register(GIE);
		return receive_readptr;
	} else {
		__bis_SR_register(GIE);
		return NULL;
	}
}
