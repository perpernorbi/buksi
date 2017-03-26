/*
 * halt.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>

void halt()
{
	__bic_SR_register(GIE);
	P1OUT |= BIT6;
	__bis_SR_register(LPM1);

}
