/*
 * drive.h
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#ifndef DRIVE_H_
#define DRIVE_H_

typedef enum {DRIVE_VELOCITY, DRIVE_DIRECT} drive_mode;

void drive_setVelocity(const char velocities[2], drive_mode mode);
const signed char* drive_getVelocities();
void drive_tick();

#endif /* DRIVE_H_ */
