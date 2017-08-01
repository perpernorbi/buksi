/*
 * adc.c
 *
 *  Created on: 2017.04.17.
 *      Author: Norbi
 */

#include <msp430.h>
#include "adc.h"


void adc_initialize()
{
	ADC10CTL0 |= ADC10SC;
}

unsigned int adc_get_battery_voltage()
{
	unsigned int voltage = ADC10MEM;
	voltage = (voltage << 1) + (voltage >> 1);
	voltage += (voltage << 1);
	voltage = (voltage >> 2);
	return voltage;
}
