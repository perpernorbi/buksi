/*
 * drive.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>

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

