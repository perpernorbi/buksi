/*
 * tick.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>
#include "tick.h"

static char tick = 0;

volatile unsigned int tick_counter = 0;

void tick_set()
{
	tick = 1;
}

int tick_get_and_clear()
{
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	char retval = tick;
	tick = 0;
	__bis_SR_register(gie);
	++tick_counter;
    return retval;
}
