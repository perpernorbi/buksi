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
#define BUFFER_SIZE 10
static char buffer[BUFFER_SIZE];
static const char* readptr;
static char* writeptr;

void serial_initialize()
{
	readptr = buffer-1;
	writeptr = buffer;
}

void serial_receiveByte(char data)
{
	__bic_SR_register(GIE);
	(*writeptr) = data;
	++writeptr;
	if (writeptr - buffer > BUFFER_SIZE) halt();
	__bis_SR_register(GIE);
}

const char * serial_getNextFrame()
{
	__bic_SR_register(GIE);
	++readptr;
	while (((*readptr) & 0x80 != 0x80) && (readptr != writeptr)) ++readptr;
	if (readptr == writeptr) {
		serial_initialize();
		__bis_SR_register(GIE);
		return NULL;
	}
	if (readptr + FRAME_SIZE <= writeptr) {
		__bis_SR_register(GIE);
		return readptr;
	} else {
		__bis_SR_register(GIE);
		return NULL;
	}
}
