#include <msp430.h>
#include <ti/mcu/msp430/Grace.h>
#include "serial.h"
#include "tick.h"
#include "drive.h"

int main(void)
{
	Grace_init();
	serial_initialize();
    while (1) {
    	__bis_SR_register(CPUOFF + GIE);

    	if (tick_get_and_clear()) {
        	drive_tick();
    		const char * dataframe = serial_getNextFrame();
    		if (dataframe)
    			drive_setVelocity(dataframe+1);
    	}
    }
    return (0);
}
