#include <msp430.h>
#include <ti/mcu/msp430/Grace.h>
#include "serial.h"
#include "tick.h"

int main(void)
{
	int i = 0;
	char wheel = 0;
	Grace_init();
	serial_initialize();
    while (1) {
    	__bis_SR_register(CPUOFF + GIE);
    	if (tick_get_and_clear()) {
    		++i;
    		if (i == 1000) {
    		    while (!(IFG2 & UCA0TXIFG)); // Poll TXIFG to until set
    		    UCA0TXBUF = '.';       // TX -> RXed character
    		    i = 0;
    		}
    		const char * dataframe = serial_getNextFrame();
    		if (dataframe) {
    			unsigned int pwm;
    			pwm = (dataframe[1] & 0x7F) << 8;
   				TA1CCR1 = pwm;
    			pwm = (dataframe[2] & 0x7F) << 8;
   				TA1CCR2 = pwm;
    		}
    	}
/*    	if (P2IN & BIT5) wheel |= 0x01;
    	if (!(i & 0x03)) {
    	    while (!(IFG2 & UCA0TXIFG)); // Poll TXIFG to until set
    	    UCA0TXBUF = wheel;
    	}
    	wheel <<= 1;
    	++i;*/

    }
    
    return (0);
}
