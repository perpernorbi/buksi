/*
 * leds.c
 *
 *  Created on: 2017.04.11.
 *      Author: Norbi
 */

#include <msp430.h>


void leds_set(char leds)
{
	char p2 = leds & (BIT7 | BIT6);
	char p1 = leds & (BIT0);
	P1OUT |= p1;
	P2OUT |= p2;
}

void leds_clear(char leds)
{
	char p2 = leds & (BIT7 | BIT6);
	char p1 = leds & (BIT0);
	P1OUT &= ~p1;
	P2OUT &= ~p2;
}
