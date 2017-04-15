#include <msp430.h>
#include <ti/mcu/msp430/Grace.h>
#include "serial.h"
#include "tick.h"
#include "drive.h"
#include "leds.h"

static const char velocityFrame = 0x80;
static const char ledFrame = 0x81;

int main(void)
{
	Grace_init();
	serial_initialize();
//	serial_test();
//	return 0;
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
    }
    return (0);
}
