/*
 * serial.h
 *
 *  Created on: 2017.03.26.
 *      Author: Norbi
 */

#ifndef SERIAL_H_
#define SERIAL_H_


void serial_initialize();
void serial_receiveByte(char data);
const char * serial_getNextFrame();

#endif /* SERIAL_H_ */
