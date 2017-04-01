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
static volatile unsigned int * const wheel_drive_pwm_reg[WHEEL_COUNT] = { &TA1CCR1, &TA1CCR2 };
static const char wheel_drive_direction_pin[WHEEL_COUNT] = { BIT2, BIT3 };
static unsigned char wheel_encoder_buffer[WHEEL_COUNT] = { 0, 0 };
static unsigned char wheel_encoder_status[WHEEL_COUNT] = { 0, 0 };

static int wheel_position[WHEEL_COUNT] = { 0, 0 };
static int wheel_velocity_last_position[WHEEL_COUNT] = { 0, 0 };
static int wheel_measured_velocity_reciprocal[WHEEL_COUNT] = { 0, 0 };

static const unsigned int speed_update_interval_mask = 0x1FF;

void drive_setVelocity(const char velocities[2])
{
	unsigned char i;
	for (i = 0; i < 2; ++i) {
		char velocity = velocities[i];
		if (velocity > 0x80) {
			(*wheel_drive_port[i]) |= wheel_drive_direction_pin[i];
			velocity = 0xFF - velocity;
		} else {
			(*wheel_drive_port[i]) &= ~wheel_drive_direction_pin[i];
		}
		(*wheel_drive_pwm_reg[i]) = (velocity & 0x7F) << 8;
	}
}

static void update_measured_velocity(unsigned char wheel_id)
{
	wheel_measured_velocity_reciprocal[wheel_id] = wheel_position[wheel_id] - wheel_velocity_last_position[wheel_id];
	wheel_velocity_last_position[wheel_id] = wheel_position[wheel_id];
	serial_sendChar((char)(wheel_measured_velocity_reciprocal[wheel_id] & 0xFF));
	serial_sendChar((char)(wheel_measured_velocity_reciprocal[wheel_id] >> 8));
}

static void encode(size_t wheel_id)
{
	if ((*wheel_encoder_port[wheel_id]) & wheel_encoder_pin[wheel_id]) wheel_encoder_buffer[wheel_id] |= 0x01;
	if ((wheel_encoder_status[wheel_id] == 1) && ((wheel_encoder_buffer[wheel_id] & 0x0F) == 0x00)) {
		wheel_encoder_status[wheel_id] = 0;
		++wheel_position[wheel_id];
	}
	if ((wheel_encoder_status[wheel_id] == 0) && (wheel_encoder_buffer[wheel_id] & 0x0F == 0x0F)) {
		wheel_encoder_status[wheel_id] = 1;
		++wheel_position[wheel_id];
	}
	wheel_encoder_buffer[wheel_id] <<= 1;

}

void drive_tick()
{
	encode(0);
	//encode(1);
	if (! (tick_counter & speed_update_interval_mask)) {
		update_measured_velocity(0);
		//update_measured_speed(1);
	}
}
