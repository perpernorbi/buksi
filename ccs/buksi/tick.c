/*
 * tick.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>

static char tick = 0;

void tick_set()
{
	tick = 1;
}

int tick_get_and_clear()
{
	__bic_SR_register(GIE);
	char retval = tick;
	tick = 0;
	__bis_SR_register(GIE);
    return retval;
}
