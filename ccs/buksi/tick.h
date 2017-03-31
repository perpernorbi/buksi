/*
 * tick.h
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#ifndef TICK_H_
#define TICK_H_

extern volatile unsigned int tick_counter;
void tick_set();
int tick_get_and_clear();


#endif /* TICK_H_ */
