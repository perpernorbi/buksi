#include <msp430.h>
#include <ti/mcu/msp430/Grace.h>
#include "serial.h"
#include "tick.h"
#include "drive.h"
#include "leds.h"
#include "adc.h"

static const char velocityFrame = 0x80;
static const char ledFrame = 0x81;

void send_status_over_uart()
{
	__bic_SR_register(GIE);

	unsigned int voltage = adc_get_battery_voltage();
	serial_sendChar(0x80);
	serial_sendChar(voltage & 0xFF);
	serial_sendChar(voltage >> 8);
	serial_sendChar(drive_getVelocities()[0]);
	serial_sendChar(drive_getVelocities()[1]);
}

int main(void)
{
	Grace_init();
	serial_initialize();
	adc_initialize();
    while (1) {
    	__bis_SR_register(CPUOFF + GIE);

    	if (tick_get_and_clear()) {
        	drive_tick();
    		const char * dataframe;
    		while (dataframe = serial_getNextFrame())
    			if (dataframe[0] == velocityFrame)
    				drive_setVelocity(dataframe+1);
    			else if (dataframe[0] == ledFrame) {
    				leds_set(dataframe[1]);
    				leds_clear(dataframe[2]);
    			}
    	}
    	if ((tick_counter & 0x3FF) == 0x3FF)
    		send_status_over_uart();

    }
    return (0);
}
