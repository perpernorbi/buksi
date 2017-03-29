/*
 * drive.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>
#include <stddef.h>

#include "serial.h"

#define WHEEL_COUNT 2
static volatile unsigned char const *wheel_encoder_port[WHEEL_COUNT] = { &P2IN, &P2IN };
static const char wheel_encoder_pin[WHEEL_COUNT] = { BIT5, BIT0 };
static unsigned char wheel_encoder_buffer[WHEEL_COUNT] = { 0, 0 };
static unsigned char wheel_encoder_status[WHEEL_COUNT] = { 0, 0 };

void drive_setVelocity(char left, char right)
{
	if (left > 0x80) {
		P2OUT |= BIT2;
		left = 0xFF - left;
	}
	else P2OUT &= ~BIT2;
	if (right > 0x80) {
		P2OUT |= BIT3;
		right = 0xFF - right;
	}
	else P2OUT &= ~BIT3;
	TA1CCR1 = (left & 0x7F) << 8;
	TA1CCR2 = (right & 0x7F) << 8;
}

static void encode(size_t wheel_id)
{
	if ((*wheel_encoder_port[wheel_id]) & wheel_encoder_pin[wheel_id]) wheel_encoder_buffer[wheel_id] |= 0x01;
	if ((wheel_encoder_status[wheel_id] == 1) && ((wheel_encoder_buffer[wheel_id] & 0x0F) == 0x00)) {
		wheel_encoder_status[wheel_id] = 0;
		serial_sendChar('d');
		serial_sendChar('o');
		serial_sendChar('w');
		serial_sendChar('n');
		serial_sendChar('\r');
	}
	if ((wheel_encoder_status[wheel_id] == 0) && (wheel_encoder_buffer[wheel_id] & 0x0F == 0x0F)) {
		wheel_encoder_status[wheel_id] = 1;
		serial_sendChar('u');
		serial_sendChar('p');
		serial_sendChar('\r');
	}
	wheel_encoder_buffer[wheel_id] <<= 1;

}

void drive_tick(unsigned int tick_counter)
{
	encode(0);
	//encode(1);
}
