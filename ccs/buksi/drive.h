/*
 * drive.h
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#ifndef DRIVE_H_
#define DRIVE_H_

void drive_setVelocity(const char velocities[2]);
void drive_tick(unsigned int tick_counter);

#endif /* DRIVE_H_ */
