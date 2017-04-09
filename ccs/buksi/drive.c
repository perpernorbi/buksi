/*
 * drive.c
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#include <msp430.h>
#include <stddef.h>

#include "serial.h"
#include "tick.h"

#define WHEEL_COUNT 2
static volatile unsigned char const * const wheel_encoder_port[WHEEL_COUNT] = { &P2IN, &P2IN };
static const char wheel_encoder_pin[WHEEL_COUNT] = { BIT5, BIT0 };
static volatile unsigned char * const wheel_drive_port[WHEEL_COUNT] = { &P2OUT, &P2OUT };
static volatile unsigned int * const wheel_drive_pwm_reg[WHEEL_COUNT] = { &TA1CCR2, &TA1CCR1 };
static const char wheel_drive_direction_pin[WHEEL_COUNT] = { BIT3, BIT2 };
static unsigned char wheel_encoder_buffer[WHEEL_COUNT] = { 0, 0 };
static unsigned char wheel_encoder_status[WHEEL_COUNT] = { 0, 0 };

static int wheel_position[WHEEL_COUNT] = { 0, 0 };
static int wheel_velocity_last_position[WHEEL_COUNT] = { 0, 0 };
static signed char wheel_measured_velocity[WHEEL_COUNT] = { 0, 0 };
static signed char wheel_desired_velocity[WHEEL_COUNT] = { 0, 0 };

static signed int controller_integrator[WHEEL_COUNT] = { 0, 0 };

static const unsigned int speed_update_interval_mask = 0xFF;

static const float controller_P = 1;
static const float controller_I = 0.8;

void drive_setVelocity(const char velocities[2])
{
	unsigned char i;
	for (i = 0; i < 2; ++i)
		wheel_desired_velocity[i] = velocities[i];
		//Dimension conversion must be done here, if necessary
}

static void update_measured_velocity(unsigned char wheel_id)
{
	wheel_measured_velocity[wheel_id] = wheel_position[wheel_id] - wheel_velocity_last_position[wheel_id];
	wheel_velocity_last_position[wheel_id] = wheel_position[wheel_id];
	serial_sendChar(wheel_measured_velocity[wheel_id]);
}

static void encode(unsigned char wheel_id)
{
	if ((*wheel_encoder_port[wheel_id]) & wheel_encoder_pin[wheel_id]) wheel_encoder_buffer[wheel_id] |= 0x01;
	if ((wheel_encoder_status[wheel_id] == 1) && ((wheel_encoder_buffer[wheel_id] & 0x0F) == 0x00)) {
		wheel_encoder_status[wheel_id] = 0;
		if (wheel_desired_velocity[wheel_id] >= 0)
			++wheel_position[wheel_id];
		else
			--wheel_position[wheel_id];
	}
	if ((wheel_encoder_status[wheel_id] == 0) && (wheel_encoder_buffer[wheel_id] & 0x0F == 0x0F)) {
		wheel_encoder_status[wheel_id] = 1;
		if (wheel_desired_velocity[wheel_id] >= 0)
			++wheel_position[wheel_id];
		else
			--wheel_position[wheel_id];
	}
	wheel_encoder_buffer[wheel_id] <<= 1;

}

static void update_pwm(unsigned char wheel, char pwm)
{
	if (pwm & 0x80) {
		(*wheel_drive_port[wheel]) |= wheel_drive_direction_pin[wheel];
        pwm &= 0x7F;
    } else {
    	(*wheel_drive_port[wheel]) &= ~wheel_drive_direction_pin[wheel];
    }
	(*wheel_drive_pwm_reg[wheel]) = pwm << 8;
}

static void control_pwm(unsigned char wheel)
{
	int measured_error;
	int u;
	measured_error = wheel_desired_velocity[wheel] - wheel_measured_velocity[wheel];
	u = (controller_P * measured_error) + (controller_I * controller_integrator[wheel]);

	controller_integrator[wheel] += measured_error;
	if (controller_integrator[wheel] > 127 / controller_I)
		controller_integrator[wheel] = 127 / controller_I;
	else if (controller_integrator[wheel] < -128 / controller_I)
		controller_integrator[wheel] = -128 / controller_I;

	if (wheel_desired_velocity[wheel] == 0) {
		update_pwm(wheel, 0);
		controller_integrator[wheel] = 0;
	}
	else {
		if (u > 127) u = 127;
		if (u < -128) u = -128;
		update_pwm(wheel, u);
	}
}

//u = measured_error >> 1 + controller_integrator[wheel] >> 1;


void drive_tick()
{
	encode(0);
	encode(1);
	if (! (tick_counter & speed_update_interval_mask)) {
		update_measured_velocity(0);
		update_measured_velocity(1);
		control_pwm(0);
		control_pwm(1);
	}
}
