/*
 *  This file is automatically generated and does not require a license
 *
 *  ==== WARNING: CHANGES TO THIS GENERATED FILE WILL BE OVERWRITTEN ====
 *
 *  To make changes to the generated code, use the space between existing
 *      "USER CODE START (section: <name>)"
 *  and
 *      "USER CODE END (section: <name>)"
 *  comments, where <name> is a single word identifying the section.
 *  Only these sections will be preserved.
 *
 *  Do not move these sections within this file or change the START and
 *  END comments in any way.
 *  ==== ALL OTHER CHANGES WILL BE OVERWRITTEN WHEN IT IS REGENERATED ====
 *
 *  This file was generated from
 *      C:/ti/grace_2_20_01_12/packages/ti/mcu/msp430/csl/interrupt_vectors/InterruptVectors_init.xdt
 */
#include <msp430.h>

/* USER CODE START (section: InterruptVectors_init_c_prologue) */
/* User defined includes, defines, global variables and functions */
#include "../../tick.h"
#include "../../serial.h"
/* USER CODE END (section: InterruptVectors_init_c_prologue) */

/*
 *  ======== InterruptVectors_graceInit ========
 */
void InterruptVectors_graceInit(void)
{
}


/* Interrupt Function Prototypes */




/*
 *  ======== USCI A0/B0 TX Interrupt Handler Generation ========
 *
 * Here are several important notes on using USCI_A0/B0 TX interrupt Handler:
 * 1. User could use the following code as a template to service the interrupt
 *    handler. Just simply copy and paste it into your user definable code
 *    section.
 *  For UART and SPI configuration:

    if (IFG2 & UCA0TXIFG) {

    }
    else if (IFG2 & UCB0TXIFG) {

    }

 *  For I2C configuration:
    if (IFG2 & UCA0/B0TXIFG) {

    }
    else if (IFG2 & UCA0/B0RXIFG) {

    }


 * 2. User could also exit from low power mode and continue with main
 *    program execution by using the following instruction before exiting
 *    this interrupt handler.
 *
 *    __bic_SR_register_on_exit(LPMx_bits);
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR_HOOK(void)
{
    /* USER CODE START (section: USCI0TX_ISR_HOOK) */
    /* replace this comment with your code */
	unsigned short gie = _get_SR_register() & GIE;
	__bic_SR_register(GIE);
	const char * charToSend = serial_getNextCharToSend();
	if (charToSend)
		UCA0TXBUF = (*charToSend);
	else
		IFG2 &= (~UCA0TXIFG);
	__bic_SR_register(gie);
    /* USER CODE END (section: USCI0TX_ISR_HOOK) */
}

/*
 *  ======== USCI A0/B0 RX Interrupt Handler Generation ========
 *
 * Here are several important notes on using USCI_A0/B0 RX interrupt Handler:
 * 1. User could use the following code as a template to service the interrupt
 *    handler. Just simply copy and paste it into your user definable code
 *    section.
 *  For UART and SPI configuration:

    if (IFG2 & UCA0RXIFG) {

    }
    else if (IFG2 & UCB0RXIFG) {

    }

*  For I2C configuration:
    if (UCB0STAT & UCSTTIFG) {

    }
    else if (UCB0STAT & UCSTPIFG) {

    }
    else if (UCB0STAT & UCNACKIFG) {

    }
    else if (UCB0STAT & UCALIFG) {

    }

 * 2. User could also exit from low power mode and continue with main
 *    program execution by using the following instruction before exiting
 *    this interrupt handler.
 *
 *    __bic_SR_register_on_exit(LPMx_bits);
 */
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK(void)
{
    /* USER CODE START (section: USCI0RX_ISR_HOOK) */
	serial_receiveByte(UCA0RXBUF);
    /* USER CODE END (section: USCI0RX_ISR_HOOK) */
}

/*
 *  ======== Timer0_A3 Interrupt Service Routine ======== 
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR_HOOK(void)
{
    /* USER CODE START (section: TIMER0_A0_ISR_HOOK) */
	tick_set();
	__bic_SR_register_on_exit(CPUOFF);
	/* USER CODE END (section: TIMER0_A0_ISR_HOOK) */
}


